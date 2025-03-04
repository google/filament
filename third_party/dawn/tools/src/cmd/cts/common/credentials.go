// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

package common

import (
	"context"
	"errors"
	"fmt"

	"dawn.googlesource.com/dawn/tools/src/gerrit"
	"dawn.googlesource.com/dawn/tools/src/gitiles"
	"dawn.googlesource.com/dawn/tools/src/resultsdb"
)

// The following known remote services are not currently checked:
// - Buildbucket (tools/src/buildbucket), as all supported API calls rely
//   directly or indirectly on Buildbucket IDs which are liable to age out over
//   time, and thus are liable to randomly break the check.
// - Sheets (tools/src/cmd/cts/common/export), as the API does not appear to
//   have a way of checking if the user has edit permissions without actually
//   attempting to edit.

type CredCheckInputs struct {
	GerritConfig  *gerrit.Gerrit
	GitilesConfig *gitiles.Gitiles
	Querier       resultsdb.Querier
}

// CheckAllRequiredCredentials checks that all the services that are necessary
// for CTS tooling functionality can be accessed correctly. Specifically, this
// checks:
//   - Interacting with Gerrit
//   - Reading from Gitiles
//   - Querying ResultDB via BigQuery
func CheckAllRequiredCredentials(ctx context.Context, inputs CredCheckInputs) error {
	var err error

	if inputs.GerritConfig != nil {
		fmt.Print("Checking if Gerrit can be accessed...")
		err = checkGerritCredentials(ctx, inputs.GerritConfig)
		if err != nil {
			fmt.Println(" failed")
			return err
		}
		fmt.Println(" success")
	} else {
		fmt.Println("Gerrit information not provided, not checking")
	}

	if inputs.GitilesConfig != nil {
		fmt.Print("Checking if Gitiles can be accessed...")
		err = checkGitilesCredentials(ctx, inputs.GitilesConfig)
		if err != nil {
			fmt.Println(" failed")
			return err
		}
		fmt.Println(" success")
	} else {
		fmt.Println("Gitiles information not provided, not checking")
	}

	if inputs.Querier != nil {
		fmt.Print("Checking if ResultsDB can be queried...")
		client := inputs.Querier.(*resultsdb.BigQueryClient)
		err = checkResultDBCredentials(ctx, client)
		if err != nil {
			fmt.Println(" failed")
			return err
		}
		fmt.Println(" success")
	} else {
		fmt.Println("ResultDB information not provided, not checking")
	}

	return nil
}

// checkGerritCredentials checks that the provided Gerrit instance has the
// necessary permissions for the "dawn" project.
func checkGerritCredentials(ctx context.Context, gerritConfig *gerrit.Gerrit) error {
	project := "dawn"
	accessInfo, err := gerritConfig.ListAccessRights(ctx, project)
	if err != nil {
		return err
	}

	projectPermissions, exists := (*accessInfo)[project]
	if !exists {
		return fmt.Errorf("Project %v not in returned data", project)
	}

	if !projectPermissions.CanUpload {
		return errors.New("Do not have permission to upload to Gerrit")
	}
	if !projectPermissions.CanAdd {
		return errors.New("Do not have permission to add to existing Gerrit CLs")
	}
	if !projectPermissions.CanAddTags {
		return errors.New("Do not have permission to add tags to existing Gerrit CLs")
	}

	return nil
}

// checkGitilesCredentials checks that the provided Gitiles instance has read
// permissions.
func checkGitilesCredentials(ctx context.Context, gitilesConfig *gitiles.Gitiles) error {
	output, err := gitilesConfig.ListFiles(ctx, "HEAD", "")
	if err != nil {
		return err
	}
	if len(output) == 0 {
		return errors.New("Did not get back any data from Gitiles")
	}
	return nil
}

// checkResultDBCredentils checks that the provided BigQueryClient can
// successfully query ResultDB data.
func checkResultDBCredentials(ctx context.Context, client *resultsdb.BigQueryClient) error {
	return client.CheckIfResultDBCanBeQueried(ctx)
}
