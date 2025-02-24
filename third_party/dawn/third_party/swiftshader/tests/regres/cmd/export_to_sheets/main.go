// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// export_to_sheets updates a Google sheets document with the latest test
// results
package main

import (
	"bufio"
	"bytes"
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"strings"

	"swiftshader.googlesource.com/SwiftShader/tests/regres/consts"
	"swiftshader.googlesource.com/SwiftShader/tests/regres/git"
	"swiftshader.googlesource.com/SwiftShader/tests/regres/testlist"

	"golang.org/x/oauth2"
	"golang.org/x/oauth2/google"
	"google.golang.org/api/sheets/v4"
)

var (
	authdir       = flag.String("authdir", "~/.regres-auth", "directory to hold credentials.json and generated token")
	projectPath   = flag.String("projpath", ".", "project path")
	testListPath  = flag.String("testlist", "tests/regres/full-tests.json", "project relative path to the test list .json file")
	spreadsheetID = flag.String("spreadsheet", "1RCxbqtKNDG9rVMe_xHMapMBgzOCp24mumab73SbHtfw", "identifier of the spreadsheet to update")
)

const (
	columnGitHash = "GIT_HASH"
	columnGitDate = "GIT_DATE"
)

func main() {
	flag.Parse()

	if err := run(); err != nil {
		log.Fatalln(err)
	}
}

func run() error {
	// Load the full test list. We use this to find the test file names.
	lists, err := testlist.Load(".", *testListPath)
	if err != nil {
		return fmt.Errorf("failed to load test list: %w", err)
	}

	// Load the creditials used for editing the Google Sheets spreadsheet.
	srv, err := createSheetsService(*authdir)
	if err != nil {
		return fmt.Errorf("failed to authenticate: %w", err)
	}

	// Ensure that there is a sheet for each of the test lists.
	if err := createTestListSheets(srv, lists); err != nil {
		return fmt.Errorf("failed to create sheets: %w", err)
	}

	spreadsheet, err := srv.Spreadsheets.Get(*spreadsheetID).Do()
	if err != nil {
		return fmt.Errorf("failed to get spreadsheet: %w", err)
	}

	req := sheets.BatchUpdateValuesRequest{
		ValueInputOption: "RAW",
	}

	testListDir := filepath.Dir(filepath.Join(*projectPath, *testListPath))
	changes, err := git.Log(testListDir, 100)
	if err != nil {
		return fmt.Errorf("failed to get git changes for '%v': %w", testListDir, err)
	}

	for _, group := range lists {
		sheetName := group.Name
		fmt.Println("Processing sheet", sheetName)
		sheet := getSheet(spreadsheet, sheetName)
		if sheet == nil {
			return fmt.Errorf("sheet '%v' not found: %w", sheetName, err)
		}

		columnHeaders, err := fetchRow(srv, spreadsheet, sheet, 0)
		if err != nil {
			return fmt.Errorf("failed to get sheet '%v' column headers: %w", sheetName, err)
		}

		columnIndices := listToMap(columnHeaders)

		hashColumnIndex, found := columnIndices[columnGitHash]
		if !found {
			return fmt.Errorf("failed to find sheet '%v' column header '%v': %w", sheetName, columnGitHash, err)
		}

		hashValues, err := fetchColumn(srv, spreadsheet, sheet, hashColumnIndex)
		if err != nil {
			return fmt.Errorf("failed to get sheet '%v' column headers: %w", sheetName, err)
		}
		hashValues = hashValues[1:] // Skip header

		hashIndices := listToMap(hashValues)
		rowValues := map[string]interface{}{}

		rowInsertionPoint := 1 + len(hashValues)

		for i := len(changes) - 1; i >= 0; i-- {
			change := changes[i]
			if !strings.HasPrefix(change.Subject, consts.TestListUpdateCommitSubjectPrefix) {
				continue
			}

			hash := change.Hash.String()
			if _, found := hashIndices[hash]; found {
				continue // Already in the sheet
			}

			rowValues[columnGitHash] = change.Hash.String()
			rowValues[columnGitDate] = change.Date.Format("2006-01-02")

			path := filepath.Join(*projectPath, group.File)
			hasData := false
			for _, status := range testlist.Statuses {
				path := testlist.FilePathWithStatus(path, status)
				data, err := git.Show(path, hash)
				if err != nil {
					continue
				}
				lines, err := countLines(data)
				if err != nil {
					return fmt.Errorf("failed to count lines in file '%s': %w", path, err)
				}

				rowValues[string(status)] = lines
				hasData = true
			}

			if !hasData {
				continue
			}

			data, err := mapToList(columnIndices, rowValues)
			if err != nil {
				return fmt.Errorf("failed to map row values to column for sheet %v. Column headers: [%+v]: %w", sheetName, columnHeaders, err)
			}

			req.Data = append(req.Data, &sheets.ValueRange{
				Range:  rowRange(rowInsertionPoint, sheet),
				Values: [][]interface{}{data},
			})
			rowInsertionPoint++

			fmt.Printf("Adding test data at %v to %v\n", hash[:8], sheetName)
		}
	}

	if _, err := srv.Spreadsheets.Values.BatchUpdate(*spreadsheetID, &req).Do(); err != nil {
		return fmt.Errorf("Values.BatchUpdate() failed: %w", err)
	}

	return nil
}

// listToMap returns the list l as a map where the key is the stringification
// of the element, and the value is the element index.
func listToMap(l []interface{}) map[string]int {
	out := map[string]int{}
	for i, v := range l {
		out[fmt.Sprint(v)] = i
	}
	return out
}

// mapToList transforms the two maps into a single slice of values.
// indices is a map of identifier to output slice element index.
// values is a map of identifier to value.
func mapToList(indices map[string]int, values map[string]interface{}) ([]interface{}, error) {
	out := []interface{}{}
	for name, value := range values {
		index, ok := indices[name]
		if !ok {
			return nil, fmt.Errorf("No index for '%v'", name)
		}
		for len(out) <= index {
			out = append(out, nil)
		}
		out[index] = value
	}
	return out, nil
}

// countLines returns the number of new lines in the byte slice data.
func countLines(data []byte) (int, error) {
	scanner := bufio.NewScanner(bytes.NewReader(data))
	lines := 0
	for scanner.Scan() {
		lines++
	}
	return lines, nil
}

// getSheet returns the sheet with the given title name, or nil if the sheet
// cannot be found.
func getSheet(spreadsheet *sheets.Spreadsheet, name string) *sheets.Sheet {
	for _, sheet := range spreadsheet.Sheets {
		if sheet.Properties.Title == name {
			return sheet
		}
	}
	return nil
}

// rowRange returns a sheets range ("name!Ai:i") for the entire row with the
// given index.
func rowRange(index int, sheet *sheets.Sheet) string {
	return fmt.Sprintf("%v!A%v:%v", sheet.Properties.Title, index+1, index+1)
}

// columnRange returns a sheets range ("name!i1:i") for the entire column with
// the given index.
func columnRange(index int, sheet *sheets.Sheet) string {
	col := 'A' + index
	if index > 25 {
		panic("UNIMPLEMENTED")
	}
	return fmt.Sprintf("%v!%c1:%c", sheet.Properties.Title, col, col)
}

// fetchRow returns all the values in the given sheet's row.
func fetchRow(srv *sheets.Service, spreadsheet *sheets.Spreadsheet, sheet *sheets.Sheet, row int) ([]interface{}, error) {
	rng := rowRange(row, sheet)
	data, err := srv.Spreadsheets.Values.Get(spreadsheet.SpreadsheetId, rng).Do()
	if err != nil {
		return nil, fmt.Errorf("failed to fetch %v: %w", rng, err)
	}
	return data.Values[0], nil
}

// fetchColumn returns all the values in the given sheet's column.
func fetchColumn(srv *sheets.Service, spreadsheet *sheets.Spreadsheet, sheet *sheets.Sheet, row int) ([]interface{}, error) {
	rng := columnRange(row, sheet)
	data, err := srv.Spreadsheets.Values.Get(spreadsheet.SpreadsheetId, rng).Do()
	if err != nil {
		return nil, fmt.Errorf("failed to fetch %v: %w", rng, err)
	}
	out := make([]interface{}, len(data.Values))
	for i, l := range data.Values {
		if len(l) > 0 {
			out[i] = l[0]
		}
	}
	return out, nil
}

// insertRows inserts blank rows into the given sheet.
func insertRows(srv *sheets.Service, spreadsheet *sheets.Spreadsheet, sheet *sheets.Sheet, aboveRow, count int) error {
	req := sheets.BatchUpdateSpreadsheetRequest{
		Requests: []*sheets.Request{{
			InsertRange: &sheets.InsertRangeRequest{
				Range: &sheets.GridRange{
					SheetId:       sheet.Properties.SheetId,
					StartRowIndex: int64(aboveRow),
					EndRowIndex:   int64(aboveRow + count),
				},
				ShiftDimension: "ROWS",
			}},
		},
	}
	if _, err := srv.Spreadsheets.BatchUpdate(*spreadsheetID, &req).Do(); err != nil {
		return fmt.Errorf("Spreadsheets.BatchUpdate() failed: %w", err)
	}
	return nil
}

// createTestListSheets adds a new sheet for each of the test lists, if they
// do not already exist. These new sheets are populated with column headers.
func createTestListSheets(srv *sheets.Service, testlists testlist.Lists) error {
	spreadsheet, err := srv.Spreadsheets.Get(*spreadsheetID).Do()
	if err != nil {
		return fmt.Errorf("failed to get spreadsheet: %w", err)
	}

	spreadsheetReq := sheets.BatchUpdateSpreadsheetRequest{}
	updateReq := sheets.BatchUpdateValuesRequest{ValueInputOption: "RAW"}
	headers := []interface{}{columnGitHash, columnGitDate}
	for _, s := range testlist.Statuses {
		headers = append(headers, string(s))
	}

	for _, group := range testlists {
		name := group.Name
		if getSheet(spreadsheet, name) == nil {
			spreadsheetReq.Requests = append(spreadsheetReq.Requests, &sheets.Request{
				AddSheet: &sheets.AddSheetRequest{
					Properties: &sheets.SheetProperties{
						Title: name,
					},
				},
			})
			updateReq.Data = append(updateReq.Data,
				&sheets.ValueRange{
					Range:  name + "!A1:Z",
					Values: [][]interface{}{headers},
				},
			)
		}
	}

	if len(spreadsheetReq.Requests) > 0 {
		if _, err := srv.Spreadsheets.BatchUpdate(*spreadsheetID, &spreadsheetReq).Do(); err != nil {
			return fmt.Errorf("Spreadsheets.BatchUpdate() failed: %w", err)
		}
	}
	if len(updateReq.Data) > 0 {
		if _, err := srv.Spreadsheets.Values.BatchUpdate(*spreadsheetID, &updateReq).Do(); err != nil {
			return fmt.Errorf("Values.BatchUpdate() failed: %w", err)
		}
	}

	return nil
}

// createSheetsService creates a new Google Sheets service using the credentials
// in the credentials.json file.
func createSheetsService(authdir string) (*sheets.Service, error) {
	authdir = os.ExpandEnv(authdir)
	if home, err := os.UserHomeDir(); err == nil {
		authdir = strings.ReplaceAll(authdir, "~", home)
	}

	os.MkdirAll(authdir, 0777)

	credentialsPath := filepath.Join(authdir, "credentials.json")
	b, err := ioutil.ReadFile(credentialsPath)
	if err != nil {
		return nil, fmt.Errorf("Unable to read client secret file '%v'\n"+
			"Obtain this file from: https://console.developers.google.com/apis/credentials", credentialsPath)
	}

	config, err := google.ConfigFromJSON(b, "https://www.googleapis.com/auth/spreadsheets")
	if err != nil {
		return nil, fmt.Errorf("failed to parse client secret file to config: %w", err)
	}

	client, err := getClient(authdir, config)
	if err != nil {
		return nil, fmt.Errorf("failed to obtain client: %w", err)
	}

	srv, err := sheets.New(client)
	if err != nil {
		return nil, fmt.Errorf("failed to to retrieve Sheets client: %w", err)
	}
	return srv, nil
}

// Retrieve a token, saves the token, then returns the generated client.
func getClient(authdir string, config *oauth2.Config) (*http.Client, error) {
	// The file token.json stores the user's access and refresh tokens, and is
	// created automatically when the authorization flow completes for the first
	// time.
	tokFile := filepath.Join(authdir, "token.json")
	tok, err := tokenFromFile(tokFile)
	if err != nil {
		tok, err = getTokenFromWeb(config)
		if err != nil {
			return nil, fmt.Errorf("failed to get token from web: %w", err)
		}
		if err := saveToken(tokFile, tok); err != nil {
			log.Printf("Warning: failed to write token: %v", err)
		}
	}
	return config.Client(context.Background(), tok), nil
}

// Request a token from the web, then returns the retrieved token.
func getTokenFromWeb(config *oauth2.Config) (*oauth2.Token, error) {
	authURL := config.AuthCodeURL("state-token", oauth2.AccessTypeOffline)
	fmt.Printf("Go to the following link in your browser then type the "+
		"authorization code: \n%v\n", authURL)

	var authCode string
	if _, err := fmt.Scan(&authCode); err != nil {
		return nil, fmt.Errorf("failed to read authorization code: %w", err)
	}

	tok, err := config.Exchange(context.TODO(), authCode)
	if err != nil {
		return nil, fmt.Errorf("failed to retrieve token from web: %w", err)
	}
	return tok, nil
}

// Retrieves a token from a local file.
func tokenFromFile(path string) (*oauth2.Token, error) {
	f, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer f.Close()
	tok := &oauth2.Token{}
	err = json.NewDecoder(f).Decode(tok)
	return tok, err
}

// Saves a token to a file path.
func saveToken(path string, token *oauth2.Token) error {
	fmt.Printf("Saving credential file to: %s\n", path)
	f, err := os.OpenFile(path, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0600)
	if err != nil {
		return fmt.Errorf("failed to cache oauth token: %w", err)
	}
	defer f.Close()
	json.NewEncoder(f).Encode(token)
	return nil
}
