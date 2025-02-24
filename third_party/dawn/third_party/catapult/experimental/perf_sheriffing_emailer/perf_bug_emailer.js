/**
 * This apps script emails the owners of open perf bugs to remind
 * them to take a look. This script is runnable at
 * https://script.google.com/a/google.com/d/1t1IEo_ymHd6TX_fumASf7zV3MFb3ysettF05b2qp5kPpiQG0AF0Ph0Mf/edit
 * and the source code lives in catapult at
 * /experimental/perf_sheriffing_emailer/perf_bug_emailer.js
 */

function authCallback(request) {
    return MonorailAPI.authCallback(request);
  }

  function getBugOwners() {
    var issues = MonorailAPI.getIssues(
        'chromium',
        'Performance=Sheriff Type:Bug modified-before:today-6',
        'open',
        1000);
    var owners = {};
    for (var i = 0; i < issues.length; i++) {
      var issue = issues[i];
      if (!issue.owner) {
        continue;
      }
      if (issue.summary.indexOf('improvement') != -1) {
        // No need to ping on improvements.
        continue;
      }
      if (!owners[issue.owner]) {
        owners[issue.owner] = [];
      }
      owners[issue.owner].push({
        'summary': issue.summary,
        'id': issue.id,
        'updated': new Date(issue.updated)
      });
    }
    return owners;
  }

  function formatDate(d) {
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

  function formatNumber(num, kind) {
    var suffix = (num == 1) ? '' : 's';
    return num + ' ' + kind + suffix;
  }

  function emailOwners() {
    var owners = getBugOwners();
    for (var owner in owners) {
      if (owner.indexOf('roll') != -1) {
        // TODO(sullivan): find a better way to deal with autorollers!!
        continue;
      }
      var bugs = owners[owner];
      bugs.sort(function(a, b) {
        return a.updated - b.updated;
      });
      var recipient = owner;
      var subject = 'Please take a look at the performance regression bugs assigned to you!';
      var body, htmlBody;
      body = htmlBody = 'You have ' + formatNumber(bugs.length, 'perf regression bug') +
                        ' assigned to you. It\'s important to address these promptly ' +
                        'in order to keep Chrome fast! ';
      htmlBody += '<br><br>Please take a look. If you need help, the documentation is at ' +
                  'http://g.co/ChromePerformanceRegressions';
      body += 'Please take a look. If you need help, the documentation is at ' +
              'http://g.co/ChromePerformanceRegressions';
      body += '\n\n';
      htmlBody += '<br><br><table border="1" cellspacing="0" cellpadding="5">' +
                  '<tr><th align="left">Bug</th><th align="left">Last Update</th></tr>';
      for (var i = 0; i < bugs.length; i++) {
        body += 'http://crbug.com/' +
                bugs[i].id + ': ' + bugs[i].summary +
                ' (last update ' + formatDate(bugs[i].updated) + ')\n';
        htmlBody += '<tr><td><a href="http://crbug.com/' +
                    bugs[i].id + '">' +
                    bugs[i].summary +
                    '</a></td><td>' +
                    formatDate(bugs[i].updated) +
                    '</td></tr>';
      }
      htmlBody += '</table><br><br>If you have feedback on how we could make perf regression ' +
                  'bugs better, please use this form: https://goo.gl/forms/0m1mYRvpR6KpdoY63';
      body += '\n\nIf you have feedback on how we could make perf regression bugs better, ' +
              'please use this form: https://goo.gl/forms/0m1mYRvpR6KpdoY63';
      MailApp.sendEmail(recipient, subject, body, {'htmlBody': htmlBody});
    }
  }
