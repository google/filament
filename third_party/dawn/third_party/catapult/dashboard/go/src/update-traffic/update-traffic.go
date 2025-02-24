// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Program update-traffic automates some chromeperf deployment tasks for
// oncall rotation.
package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"sort"

	"google.golang.org/api/appengine/v1"
)

var (
	trafficYamls = map[string]string{
		"api":                "dashboard/cloudbuild_traffic/api.yaml",
		"default":            "dashboard/cloudbuild_traffic/default.yaml",
		"perf-issue-service": "perf_issue_service/cloudbuild-perf-issue-service-traffic.yaml",
		"pinpoint":           "dashboard/cloudbuild_traffic/pinpoint.yaml",
		"skia-bridge":        "skia_bridge/skia-bridge-traffic.yaml",
		"upload-processing":  "dashboard/cloudbuild_traffic/upload-processing.yaml",
		"upload":             "dashboard/cloudbuild_traffic/upload.yaml",
	}
	app          = flag.String("app", "chromeperf", "GAE project app name")
	serviceId    = flag.String("service-id", "", "check/update only this service")
	checkoutBase = flag.String("checkout-base", "", "root directory for catapult repo checkout")
)

func gitHash(id string) string {
	return id[len("cloud-build-"):]
}

// Sort Versions by CreateTime such that versions[0] is the latest.
type byCreateTime []*appengine.Version

func (s byCreateTime) Len() int           { return len(s) }
func (s byCreateTime) Swap(i, j int)      { s[i], s[j] = s[j], s[i] }
func (s byCreateTime) Less(i, j int) bool { return s[i].CreateTime > s[j].CreateTime }

func liveVersion(service *appengine.Service, versions []*appengine.Version) (*appengine.Version, error) {
	nonzeroSplits := []string{}
	for _, v := range versions {
		split := service.Split.Allocations[v.Id]
		if split == 1.0 {
			return v, nil
		}
		if split > 0 {
			nonzeroSplits = append(nonzeroSplits, fmt.Sprintf("%q: %v", v.Id, split))
		}
	}
	return nil, fmt.Errorf("cannot determined which %q version is live; no version has full traffic allocation but the following versions each had non-zero traffic: %v", service.Id, nonzeroSplits)
}

type serviceUpdate struct {
	serviceId        string
	fromHash, toHash string
	fromDate, toDate string
}

func main() {
	flag.Parse()

	ctx := context.Background()
	appengineService, err := appengine.NewService(ctx)
	if err != nil {
		panic(err)
	}
	serviceListCall := appengineService.Apps.Services.List(*app)
	serviceListResp, err := serviceListCall.Do()
	if err != nil {
		panic(err)
	}

	updates := []serviceUpdate{}
	for _, service := range serviceListResp.Services {
		if *serviceId != "" && service.Id != *serviceId {
			continue
		}
		versionsListCall := appengineService.Apps.Services.Versions.List(*app, service.Id)
		versionsListResp, err := versionsListCall.Do()
		if err != nil {
			panic(err)
		}
		versions := versionsListResp.Versions
		live, err := liveVersion(service, versions)
		if err != nil {
			panic(err)
		}
		sort.Sort(byCreateTime(versions))
		latest := versions[0]

		updates = append(updates, serviceUpdate{
			serviceId: service.Id,
			fromHash:  gitHash(live.Id),
			fromDate:  live.CreateTime,
			toHash:    gitHash(latest.Id),
			toDate:    latest.CreateTime,
		})
	}

	re := regexp.MustCompile(`_SERVICE_VERSION: 'cloud-build-.*'`)

	for _, update := range updates {
		fmt.Printf("checking app %q service %q\n", *app, update.serviceId)
		current := update.fromHash == update.toHash
		trafficYaml, ok := trafficYamls[update.serviceId]
		if !ok {
			fmt.Printf("\tunrecognized service, no known yaml file\n")
			continue
		}
		if current {
			fmt.Printf("\tUP-TO-DATE\t%s\t(%s)\n", update.toHash, update.toDate)
		} else {
			fmt.Printf("\tNEEDS UPDATE\t Current: %s (%s)\tLatest available: %s (%s)\n", update.fromHash, update.fromDate, update.toHash, update.toDate)
			fmt.Printf("\tGitiles URL: https://chromium.googlesource.com/catapult.git/+log/%s..%s\n", update.fromHash, update.toHash)
			if *checkoutBase == "" {
				fmt.Printf("\t[no checkout-base flag specified; skipping file edit]\n")
				continue
			}
			yamlPath := filepath.Join(*checkoutBase, trafficYaml)
			yamlBytes, err := os.ReadFile(yamlPath)
			if err != nil {
				panic(err)
			}
			yamlStr := string(yamlBytes)
			newVersionStr := fmt.Sprintf("_SERVICE_VERSION: 'cloud-build-%s'", update.toHash)
			yamlStr = re.ReplaceAllLiteralString(yamlStr, newVersionStr)
			if err := os.WriteFile(yamlPath, []byte(yamlStr), 0666); err != nil {
				panic(err)
			}
			fmt.Printf("\tupdated %s's _SERVICE_VERSION to `cloud-build-%s`\n", yamlPath, update.toHash)
		}
	}
}
