/**
 * Send the necessary HTMLSerializer properties back to the extension.
 *
 * @param {HTMLSerializer} htmlSerializer The HTMLSerializer.
 */
function sendHTMLSerializerToExtension(htmlSerializer) {
  chrome.runtime.sendMessage(htmlSerializer.asDict());
}

var htmlSerializer = new HTMLSerializer();
htmlSerializer.processDocument(document);
htmlSerializer.fillHolesAsync(document, sendHTMLSerializerToExtension);
