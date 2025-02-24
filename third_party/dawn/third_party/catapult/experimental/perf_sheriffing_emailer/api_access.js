/*
 * This script handles oauth api access.
 */

// You need sullivan to share this file with you to make the script work.
var DRIVE_KEYFILE_ID = '0B_lQgYV0hGoNdHdYWXgwd1JrTEk';

function getApiData(url, token) {
  var cache = CacheService.getScriptCache();
  var cached = cache.get(url);
  if (cached) {
    Logger.log('Got from cache: ' + url);
    return JSON.parse(cached);
  }

  var service = getService(getPrivateKeyDetailsFromDriveFile(DRIVE_KEYFILE_ID));
  if (service.hasAccess()) {
    Logger.log('Making request: ' + url);
    var response = UrlFetchApp.fetch(url, {
      method: 'POST',
      headers: {
        Authorization: 'Bearer ' + service.getAccessToken()
      }
    });
    var data = response.getContentText();
    if (data.length < 50000) {
      try {
        cache.put(url, data);
      } catch (e) {
        Logger.log('Value too large to cache for ' + url);
      }
    }
      return JSON.parse(data);
  } else {
    Logger.log(service.getLastError());
  }
}

/**
 * Configures the service.
 * @param {Object} privateKeyDetails Dict with private key and client email.
 */
function getService(privateKeyDetails) {
  return OAuth2.createService('PerfDash:' + Session.getActiveUser().getEmail())
      // Set the endpoint URL.
      .setTokenUrl('https://accounts.google.com/o/oauth2/token')

      // Set the private key and issuer.
      .setPrivateKey(privateKeyDetails['private_key'])
      .setIssuer(privateKeyDetails['client_email'])

      // Set the property store where authorized tokens should be persisted.
      .setPropertyStore(PropertiesService.getScriptProperties())

      // Set the scope. This must match one of the scopes configured during the
      // setup of domain-wide delegation.
      .setScope('https://www.googleapis.com/auth/userinfo.email');
}

/**
 * Parse the private key details from a file stored in Google Drive.
 * @param {string} driveFileId The id of the file in drive to parse.
 */
function getPrivateKeyDetailsFromDriveFile(driveFileId) {
  var file = DriveApp.getFileById(driveFileId);
  return JSON.parse(file.getAs('application/json').getDataAsString());
}