/**
 * This apps script produces summary statistics about perf benchmarks
 * and mails them to the owners. It depends on the api access script.
 * This script is runnable at
 * https://script.google.com/a/google.com/d/1tamTAlWH-Ox6Fbn4c4aOiMrvsD-Qk-VIg2DD-4ZvJaMUUaKsHYmnmvFE/edit
 * and the source code lives in catapult at
 * /experimental/perf_sheriffing_emailer/benchmark_owner_emailer.js
 */
function authCallback(request) {
    return MonorailAPI.authCallback(request);
  }

  function getFixedBugs() {
    var issues = MonorailAPI.getIssues(
      'chromium',
      'status:Fixed,Verified statusmodified>today-7 Performance=Sheriff',
      'all',
      1000);
    return issues;
  }

  function getWontFixBugs() {
    var issues = MonorailAPI.getIssues(
      'chromium',
      'status:WontFix,Archived statusmodified>today-7 Performance=Sheriff',
      'all',
      1000);
    return issues;
  }

  function getOpenBugs() {
    var issues = MonorailAPI.getIssues(
      'chromium',
      'Performance=Sheriff Type:Bug',
      'open',
      1000);
    return issues;
  }

  function getBenchmarkOwners() {
    var id = '1xaAo0_SU3iDfGdqDJZX_jRV0QtkufwHUKH3kQKF3YQs';
    var ss = SpreadsheetApp.openById(id);
    var sheet = ss.getSheetByName('All benchmarks');
    var data = sheet.getRange(4, 1, 200, 2).getValues();
    var benchmarkOwners = {};
    for (var i = 0; i < data.length; i++) {
      var benchmark = data[i][0];
      if (!benchmark) {
        continue;
      }
      var owner = data[i][1];
      if (!owner) {
        continue;
      }
      benchmarkOwners[benchmark] = owner;
    }
    return benchmarkOwners;
  }

  function fillBugDict(bugList, bugDict) {
    for (var i = 0; i < bugList.length; i++) {
      bugDict[bugList[i].id] = {'alerts': [], 'details': bugList[i]};
    }
  }

  function getDisabledList() {
    var repoUrl = 'https://chromium.googlesource.com/chromium/src';
    var path = 'tools/perf/expectations.config';
    var url = repoUrl + '/+/HEAD/' + path + '?format=TEXT';
    var response = UrlFetchApp.fetch(url);
    var expectations = Utilities.newBlob(Utilities.base64Decode(response.getContentText())).getDataAsString();
    var lines = expectations.split('\n');
    var benchmarks = {}
    for (var i = 0; i < lines.length; i++) {
      line = lines[i];
      if (line.indexOf('#') == 0 || /^\s*$/.test(line)) {
        // Skip comments and whitespace lines
        continue;
      }
      // From https://chromium.googlesource.com/chromium/src/+/master/docs/speed/perf_bot_sheriffing.md#Disabling-Telemetry-Tests
      // "An expectation is a line in the expectations file in the following format:
      // reason [ conditions ] benchmark/story [ Skip ]"
      // This regex grabs that format.
      var result = line.match(/((\S+)\s+)?\[\s*(.+)\s*\]\s+(\S+)\s+\[\s*(\S+)\s*\]/);
      if (!result) {
        Logger.log('Invalid line: ' + line);
        continue;
      }
      var reason = result[2];
      var bugId = undefined;
      if (reason) {
        if (reason.indexOf('http') != 0) {
          reason = 'http://' + reason;
        }
        if (reason.match(/\d+/)) {
          bugId = reason.match(/\d+/)[0]
        }
      }
      var conditions = result[3].trim();
      var benchmark = result[4].split('/')[0];
      var story = result[4].substring(result[4].indexOf('/') + 1);
      var skip = result[5];
      if (!benchmarks[benchmark]) {
        benchmarks[benchmark] = [];
      }
      benchmarks[benchmark].push({
        'reason': reason,
        'bugId': bugId,
        'conditions': conditions,
        'story': story
      });
    }
    return benchmarks;
  }

  function createDisabledTable(disabledInfo) {
    if (!disabledInfo) {
      return null;
    }
    var body = 'Story, Bug, Conditions:';
    var htmlBody = '<table border="1" cellspacing="0" cellpadding="5">' +
                   '<tr><th align="left">Story</th>' +
                   '<th align="left">Bug</th>' +
                   '<th align="left">Status</th>' +
                   '<th align="left">Owner</th>' +
                   '<th align="left">Opened</th>' +
                   '<th align="left">Last Update</th>' +
                   '<th align="left">Conditions</th></tr>';
    disabledInfo.sort(function(a, b) {
      return a.bugId - b.bugId;
    });
    for (var i = 0; i < disabledInfo.length; i++) {
      var info = disabledInfo[i];
      var issue = {};
      if (info.bugId) {
        var issue = MonorailAPI.getIssue('chromium', info.bugId);
      }
      body += info.story + ', ' + info.reason + ', ' + info.conditions;
      htmlBody += '<tr><td>' + info.story + '</td>' +
                  '<td><a href="' + info.reason + '">' + (info.bugId || '') + '</a></td>' +
                  '<td>' + (issue.status || '') + '</td>' +
                  '<td>' + (issue.owner || '') + '</td>' +
                  '<td>' + formatDate(issue.published) + '</td>' +
                  '<td>' + formatDate(issue.updated) + '</td>' +
                  '<td>' + info.conditions + '</td></tr>';
    }
    htmlBody += '</table>';
    return {'body': body, 'htmlBody': htmlBody};
  }

  function createTable(bugs) {
    bugs.sort(function(a, b) {
      return a.details.updated - b.details.updated;
    });
    var body = '';
    var htmlBody = '<table border="1" cellspacing="0" cellpadding="5">' +
                   '<tr><th align="left">Bug</th>' +
                   '<th align="left">Status</th>' +
                   '<th align="left">Owner</th>' +
                   '<th align="left">Opened</th>' +
                   '<th align="left">Last Update</th></tr>';
    for (var i = 0; i < bugs.length; i++) {
      var bug = bugs[i].details;
      body += 'http://crbug.com/' +
              bug.id + ': ' + bug.summary +
              ': ' + bug.status + ' (last update ' + formatDate(bug.updated) + ')\n';
      htmlBody += '<tr><td><a href="http://crbug.com/' +
                  bug.id + '">' +
                  bug.summary +
                  '</a></td><td>' +
                  bug.status +
                  '</td><td>' +
                  (bug.owner || '') +
                  '</td><td>' +
                  formatDate(bug.published) +
                  '</td><td>' +
                  formatDate(bug.updated) +
                  '</td></tr>';
    }
    htmlBody += '</table>';
    return {'body': body, 'htmlBody': htmlBody};
  }

  function formatDate(d) {
    if (!d) {
      return '';
    }
    var year = d.getFullYear();
    var month = d.getMonth() + 1;
    if (month < 10) {
      month = '0' + month;
    }
    var day = d.getDate();
    if (day < 10) {
      day = '0' + day;
    }
    return year + '-' + month + '-' + day;
  }

  function emailBenchmarkOwners() {
    var owners = getBenchmarkOwners();
    var openBugs = getOpenBugs();
    var fixedBugs = getFixedBugs();
    var wontFixBugs = getWontFixBugs();
    var allBugs = {};
    fillBugDict(openBugs, allBugs);
    fillBugDict(fixedBugs, allBugs);
    fillBugDict(wontFixBugs, allBugs);

    // Use dashboard API to get each bug, and map it to benchmarks.
    // Start by getting N alerts, then pull in the ones that are missing.
    var url = 'https://chromeperf.appspot.com/api/alerts/history/80';
    var alerts = getApiData(url).anomalies;
    alerts.concat(getApiData(url + '?sheriff=V8%20Perf%20Sheriff').anomalies);
    alerts.concat(getApiData(url + '?sheriff=V8%20Memory%20Perf%20Sheriff').anomalies);
    alerts.concat(getApiData(url + '?sheriff=V8%20Telemetry%20RuntimeStats').anomalies);
    for (var i = 0; i < alerts.length; i++) {
      if (!allBugs[alerts[i].bug_id]) {
        continue;
      }
      allBugs[alerts[i].bug_id].alerts.push(alerts[i]);
    }
    for (var bugId in allBugs) {
      if (allBugs[bugId].alerts.length == 0) {
        url = 'https://chromeperf.appspot.com/api/alerts?limit=500&bug_id=' + bugId;
        allBugs[bugId].alerts = getApiData(url).anomalies;
      }
    }
    // Now create a list of bugs for each benchmark name.
    var benchmarkBugs = {};
    for (var benchmark in owners) {
      benchmarkBugs[benchmark] = {};
    }
    for (var bugId in allBugs) {
      for (var i = 0; i < allBugs[bugId].alerts.length; i++) {
        var benchmark = allBugs[bugId].alerts[i].testsuite;
        if (!benchmarkBugs[benchmark]) {
          benchmarkBugs[benchmark] = {};
        }
        benchmarkBugs[benchmark][bugId] = true;
      }
    }

    // And get a list of disabledStories for each benchmark name.
    var disabledStories = getDisabledList();

    // For each benchmark, email a chart to owners.
    for (var benchmark in benchmarkBugs) {
      var openBugs = [], fixedBugs = [], wontFixBugs = [];
      for (var bugId in benchmarkBugs[benchmark]) {
        if (!owners[benchmark]) {
          // Can't send mail if there is no owner.
          continue;
        }
        if (allBugs[bugId].details.state == 'open') {
          openBugs.push(allBugs[bugId]);
        }
        else if (allBugs[bugId].details.status == 'Fixed' || allBugs[bugId].details.status == 'Verified') {
          fixedBugs.push(allBugs[bugId]);
        }
        else if (allBugs[bugId].details.status == 'WontFix' || allBugs[bugId].details.status == 'Archived') {
          wontFixBugs.push(allBugs[bugId]);
        }
      }
      var openBugsTable = createTable(openBugs);
      var fixedBugsTable = createTable(fixedBugs);
      var wontFixBugsTable = createTable(wontFixBugs);
      var recipient = owners[benchmark];
      if (!recipient) {
        continue;
      }
      var subject = 'Summary for benchmark ' + benchmark + ' ' + formatDate(new Date());
      var body, htmlBody;
      body = htmlBody = 'This is an auto-generated summary of bugs that were open, fixed, or closed for your benchmark, ' + benchmark + ' over the past week.';
      htmlBody += '<br><br>If you are not the correct benchmark owner, please see ' +
                  '<a href="https://chromium.googlesource.com/chromium/src/+/master/docs/speed/benchmark/benchmark_ownership.md">go/change-benchmark-owner</a>.';
      body += 'If you are not the correct owner, please see go/change-benchmark-owner.';
      htmlBody += '<br><br>If you have feedback on this email, please <a href="' +
                  'https://goo.gl/forms/CJ87YLAntmDzqeXZ2">fill out our survey!</a>';
      body += '\n\nIf you have feedback on this email, please fill out our survey: https://goo.gl/forms/CJ87YLAntmDzqeXZ2';
      if (openBugs.length > 0) {
        body += '\n\nBugs that are currently open for ' + benchmark + ':\n';
        htmlBody += '<br><br>Bugs that are currently <b>open</b> for ' + benchmark + ':<br>';
        body + openBugsTable.body;
        htmlBody += openBugsTable.htmlBody;
      }
      if (fixedBugs.length > 0) {
        body += '\n\nBugs that were marked Fixed for ' + benchmark + ' last week:\n';
        htmlBody += '<br><br>Bugs that were marked <b>Fixed</b> for ' + benchmark + ' last week:<br>';
        body += fixedBugsTable.body;
        htmlBody += fixedBugsTable.htmlBody;
      }
      if (wontFixBugs.length > 0) {
        body += '\n\nBugs that were marked WontFix for ' + benchmark + ' last week:\n';
        htmlBody += '<br><br>Bugs that were marked <b>WontFix</b> for ' + benchmark + ' last week:<br>';
        body += wontFixBugsTable.body;
        htmlBody += wontFixBugsTable.htmlBody;
      }
      var disabledTable = createDisabledTable(disabledStories[benchmark]);
      if (disabledTable) {
        body += '\n\nDisabled stories for ' + benchmark + ':\n';
        htmlBody += '<br><br>Disabled stories for ' + benchmark + ':<br>';
        body += disabledTable.body;
        htmlBody += disabledTable.htmlBody;
      }
      if (wontFixBugs.length == 0 && fixedBugs.length == 0 && openBugs.length == 0 && !disabledStories[benchmark]) {
        // Don't send empty email.
        Logger.log('No bugs for ' + benchmark);
        continue;
      }
      MailApp.sendEmail(recipient, subject, body, {'htmlBody': htmlBody});
    }
  }
