#!/usr/bin/env python3

# Copyright (C) 2025 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
This script automates the process of closing a Sonatype OSSRH staging repository.

To run the script:
    python3 ./close-sonatype-staging-repository.py

To run the embedded unit tests:
    python3 ./close-sonatype-staging-repository.py test
"""

import sys
import os
import base64
import json
import pathlib
import unittest
import io
from unittest import mock
import urllib.request
import urllib.error

PROPERTIES_PATH = pathlib.Path.home() / ".gradle" / "gradle.properties"
API_BASE_URL = "https://ossrh-staging-api.central.sonatype.com/manual"

def get_gradle_credentials(file_path: pathlib.Path) -> (str, str):
    """
    Reads sonatypeUsername and sonatypePassword from the properties file.
    """
    if not file_path.exists():
        print(f"Error: Properties file not found at {file_path}", file=sys.stderr)
        sys.exit(1)

    username = None
    password = None

    try:
        with open(file_path, 'r') as f:
            for line in f:
                line = line.strip()

                # Ignore blank lines and comments
                if not line or line.startswith("#"):
                    continue

                if line.startswith("sonatypeUsername="):
                    username = line.split("=", 1)[1].strip()
                elif line.startswith("sonatypePassword="):
                    password = line.split("=", 1)[1].strip()

        if not username or not password:
            print(f"Error: 'sonatypeUsername' or 'sonatypePassword' not found in {file_path}", file=sys.stderr)
            sys.exit(1)

        return username, password

    except Exception as e:
        print(f"Error reading properties file: {e}", file=sys.stderr)
        sys.exit(1)


def generate_auth_token(username: str, password: str) -> str:
    """
    Generates a Base64 encoded auth token from username and password.
    """
    auth_string = f"{username}:{password}"
    auth_bytes = auth_string.encode('utf-8')
    token_bytes = base64.b64encode(auth_bytes)
    return token_bytes.decode('utf-8')


def find_staging_repository(token: str) -> str:
    """
    Finds the 'open' staging repository key from the Sonatype API.
    """
    print("Searching for staging repository...")

    url = f"{API_BASE_URL}/search/repositories"
    headers = {
        "Authorization": f"Bearer {token}",
        "Accept": "application/json"
    }

    req = urllib.request.Request(url, headers=headers, method="GET")

    try:
        with urllib.request.urlopen(req) as response:
            response_body = response.read().decode('utf-8')
            data = json.loads(response_body)

        repositories = data.get("repositories", [])

        open_repositories = list(filter(lambda repo: repo.get("state") == "open", repositories))

        if len(open_repositories) == 0:
            print("Error: No 'open' staging repositories found.", file=sys.stderr)
            sys.exit(1)

        if len(open_repositories) > 1:
            print(f"Error: Expected 1 'open' repository, but found {len(repositories)}.", file=sys.stderr)
            sys.exit(1)

        repo_key = open_repositories[0].get("key")
        if not repo_key:
            print("Error: Repository found, but it has no 'key'.", file=sys.stderr)
            sys.exit(1)

        return repo_key

    except urllib.error.HTTPError as e:
        # Handle HTTP errors (e.g., 401 Unauthorized, 404 Not Found, 500 Server Error)
        print(f"Error during repository search: HTTP {e.code} {e.reason}", file=sys.stderr)
        try:
            # Try to read the error response body for more context
            error_body = e.read().decode('utf-8')
            print(f"Response body: {error_body}", file=sys.stderr)
        except Exception:
            pass  # Ignore if we can't read the body
        sys.exit(1)
    except urllib.error.URLError as e:
        # Handle network errors (e.g., connection refused)
        print(f"Error during repository search: {e.reason}", file=sys.stderr)
        sys.exit(1)
    except json.JSONDecodeError:
        print("Error: Failed to decode JSON response from server.", file=sys.stderr)
        sys.exit(1)


def close_staging_repository(token: str, repo_key: str):
    """
    Closes (promotes) the staging repository with the given key.
    """
    print(f"Attempting to close staging repository: {repo_key}")

    url = f"{API_BASE_URL}/upload/repository/{repo_key}"
    headers = {"Authorization": f"Bearer {token}"}

    req = urllib.request.Request(url, headers=headers, method="POST")

    try:
        with urllib.request.urlopen(req) as response:
            print(f"Successfully submitted request. Server responded with: {response.status} {response.reason}")
            print("It may take a few moments to process.")

    except urllib.error.HTTPError as e:
        print(f"Error closing repository: HTTP {e.code} {e.reason}", file=sys.stderr)
        try:
            error_body = e.read().decode('utf-8')
            print(f"Response body: {error_body}", file=sys.stderr)
        except Exception:
            pass
        sys.exit(1)
    except urllib.error.URLError as e:
        print(f"Error closing repository: {e.reason}", file=sys.stderr)
        sys.exit(1)


def main():
    username, password = get_gradle_credentials(PROPERTIES_PATH)
    token = generate_auth_token(username, password)
    print("Successfully generated auth token.")

    repo_key = find_staging_repository(token)
    print(f"Found staging repository: {repo_key}")

    close_staging_repository(token, repo_key)

    print("\nScript completed successfully.")


# --- Unit Tests ---

class TestSonatypeScript(unittest.TestCase):

    @mock.patch('pathlib.Path.exists', return_value=True)
    @mock.patch('builtins.open', new_callable=mock.mock_open,
              read_data="# This is a comment\nsonatypeUsername=testuser\n\nsonatypePassword=testpass\nother=data")
    def test_get_gradle_credentials_success(self, mock_file, mock_exists):
        """Tests successful credential parsing, ignoring comments."""
        user, pw = get_gradle_credentials(pathlib.Path("/fake/path"))
        self.assertEqual(user, "testuser")
        self.assertEqual(pw, "testpass")

    @mock.patch('pathlib.Path.exists', return_value=False)
    def test_get_gradle_credentials_no_file(self, mock_exists):
        """Tests error exit when file is missing."""
        with self.assertRaises(SystemExit) as cm:
            get_gradle_credentials(pathlib.Path("/fake/path"))

    @mock.patch('pathlib.Path.exists', return_value=True)
    @mock.patch('builtins.open', new_callable=mock.mock_open, read_data="# Only comments\nsonatypeUsername=testuser")
    def test_get_gradle_credentials_missing_key(self, mock_file, mock_exists):
        """Tests error exit when a key is missing."""
        with self.assertRaises(SystemExit) as cm:
            get_gradle_credentials(pathlib.Path("/fake/path"))

    def test_generate_auth_token(self):
        """Tests the Base64 token encoding."""
        token = generate_auth_token("foobar", "abc123")
        # echo -n "foobar:abc123" | base64
        self.assertEqual(token, "Zm9vYmFyOmFiYzEyMw==")

    @mock.patch('urllib.request.urlopen')
    def test_find_repository_success(self, mock_urlopen):
        """Tests finding exactly one open repository."""
        mock_response = mock.Mock()
        mock_response.status = 200
        mock_response.reason = "OK"
        mock_response.read.return_value = json.dumps({
            "repositories": [{"key": "test-repo-key-123", "state": "open"}]
        }).encode('utf-8')
        mock_urlopen.return_value.__enter__.return_value = mock_response

        key = find_staging_repository("fake-token")
        self.assertEqual(key, "test-repo-key-123")

    @mock.patch('urllib.request.urlopen')
    def test_find_repository_success(self, mock_urlopen):
        """Tests finding exactly one open repository."""
        mock_response = mock.Mock()
        mock_response.status = 200
        mock_response.reason = "OK"
        mock_response.read.return_value = json.dumps({
            "repositories": [{"key": "test-repo-key-123", "state": "open"}, {"key": "test-repo-key-456", "state": "closed"}]
        }).encode('utf-8')
        mock_urlopen.return_value.__enter__.return_value = mock_response

        key = find_staging_repository("fake-token")
        self.assertEqual(key, "test-repo-key-123")

    @mock.patch('urllib.request.urlopen')
    def test_find_repository_success(self, mock_urlopen):
        """Tests error exit when only a closed repository is found."""
        mock_response = mock.Mock()
        mock_response.status = 200
        mock_response.reason = "OK"
        mock_response.read.return_value = json.dumps({
            "repositories": [{"key": "test-repo-key-123", "state": "closed"}]
        }).encode('utf-8')
        mock_urlopen.return_value.__enter__.return_value = mock_response

        with self.assertRaises(SystemExit) as cm:
            key = find_staging_repository("fake-token")

    @mock.patch('urllib.request.urlopen')
    def test_find_repository_not_found(self, mock_urlopen):
        """Tests error exit when zero repositories are found."""
        mock_response = mock.Mock()
        mock_response.status = 200
        mock_response.read.return_value = json.dumps({"repositories": []}).encode('utf-8')
        mock_urlopen.return_value.__enter__.return_value = mock_response

        with self.assertRaises(SystemExit) as cm:
            find_staging_repository("fake-token")

    @mock.patch('urllib.request.urlopen')
    def test_find_repository_too_many(self, mock_urlopen):
        """Tests error exit when multiple open repositories are found."""
        mock_response = mock.Mock()
        mock_response.status = 200
        mock_response.read.return_value = json.dumps({
            "repositories": [{"key": "key1", "state": "open"}, {"key": "key2", "state": "open"}]
        }).encode('utf-8')
        mock_urlopen.return_value.__enter__.return_value = mock_response

        with self.assertRaises(SystemExit) as cm:
            find_staging_repository("fake-token")

    @mock.patch('urllib.request.urlopen')
    def test_find_repository_http_error(self, mock_urlopen):
        """Tests error exit on a network HTTP error."""
        # Mock an HTTPError
        mock_urlopen.side_effect = urllib.error.HTTPError(
            url='http://fake.com',
            code=401,
            msg="Unauthorized",
            hdrs={},
            fp=io.BytesIO(b'{"error":"bad token"}')
        )

        with self.assertRaises(SystemExit) as cm:
            find_staging_repository("fake-token")

    @mock.patch('urllib.request.urlopen')
    def test_close_repository_success(self, mock_urlopen):
        """Tests successful repository close."""
        mock_response = mock.Mock()
        mock_response.status = 202  # 202 Accepted is a common response for POST
        mock_response.reason = "Accepted"
        mock_urlopen.return_value.__enter__.return_value = mock_response

        close_staging_repository("fake-token", "test-repo-key")

        # Check that the request object was created correctly
        called_request = mock_urlopen.call_args[0][0]
        self.assertEqual(called_request.full_url, f"{API_BASE_URL}/upload/repository/test-repo-key")
        self.assertEqual(called_request.method, "POST")
        self.assertEqual(called_request.headers["Authorization"], "Bearer fake-token")

    @mock.patch('urllib.request.urlopen')
    def test_close_repository_http_error(self, mock_urlopen):
        """Tests error exit on close network failure."""
        mock_urlopen.side_effect = urllib.error.HTTPError(
            url='http://fake.com',
            code=500,
            msg="Server Error",
            hdrs={},
            fp=io.BytesIO(b'{"error":"failed to close"}')
        )

        with self.assertRaises(SystemExit) as cm:
            close_staging_repository("fake-token", "test-repo-key")


def run_unit_tests():
    """
    Configures and runs the unit tests.
    """
    print("Running unit tests...")
    # We replace sys.argv to prevent unittest.main from
    # trying to parse the 'test' argument.
    original_argv = sys.argv
    sys.argv = [original_argv[0]]  # Keep only the script name
    unittest.main()
    sys.argv = original_argv # Restore original args


if __name__ == '__main__':
    # Check if the user wants to run tests
    if len(sys.argv) > 1 and sys.argv[1].lower() == 'test':
        run_unit_tests()
    else:
        # Run the main script logic
        main()
