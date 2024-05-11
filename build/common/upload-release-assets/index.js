/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const uploadReleaseAssets = async ({github, context}, assetPathsToUpload, releaseTag) => {
    const fs = require('fs/promises');
    const path = require('path');

    const filesToUpload = assetPathsToUpload.map(assetPath => ({
        buffer: fs.readFile(assetPath),
        assetName: path.basename(assetPath)
    }));

    const findReleaseMatchingTag = async tag => {
        const { data: releases } = await github.rest.repos.listReleases(context.repo);
        const release = releases.find(release => release.tag_name === releaseTag);
        if (!release) {
            throw new Error(`Could not locate release with tag '${releaseTag}'`);
        }
        return release;
    };

    const release = await findReleaseMatchingTag(releaseTag);

    console.log(`Found release named '${release.name}' matching tag '${release.tag_name}'.`);

    const uploadPromises = [];
    for (const file of filesToUpload) {
        console.log(`Uploading asset ${file.assetName}.`);
        const p = github.rest.repos.uploadReleaseAsset({
            ...context.repo,
            release_id: release.id,
            name: file.assetName,
            data: await file.buffer
        });
        uploadPromises.push(p);
    }

    await Promise.all(uploadPromises);
    console.log("Done!");
}

module.exports = uploadReleaseAssets;

////////////////////////////////////////////////////////////////////////////////////////////////////

// To test this script locally, uncomment the code below and run:
//
// npm install
// node index.js

// const { Octokit } = require("@octokit/rest");
// const glob = require("@actions/glob");
// (async () => {
//     const github = new Octokit({
//         auth: ""     // <-- paste GitHub auth token here
//     });
//     const context = {
//         repo: {
//             owner: "bejado",
//             repo: "filament",
//         },
//     };
//     const globber = await glob.create('*.txt');
//     await uploadReleaseAssets({ github, context }, await globber.glob(), 'vtest-28');
// })();
