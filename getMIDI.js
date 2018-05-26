var fs = require("fs");
var webdriver = require("selenium-webdriver");
// chrome = require("selenium-webdriver/firefox");
By = webdriver.By;
until = webdriver.until;
// options = new chrome.Options();
// options.addArguments("headless"); // note: without dashes
// options.addArguments("disable-gpu");
// var path = require("chromedriver").path;
var path = require("geckodriver").path;
// var service = new chrome.ServiceBuilder(path).build();
// chrome.setDefaultService(service);
var driver = new webdriver.Builder()
  .forBrowser("firefox")
  // .withCapabilities(webdriver.Capabilities.chrome())
  // .setChromeOptions(options) // note this
  .build();

var exportSelector =
  "#beepboxEditorContainer > div > div.editor-widget-column > div.editor-widgets > div.editor-controls > div.editor-menus > button:nth-child(4)";

var urls = JSON.parse(fs.readFileSync("beepboxUrls.json", "utf-8"));
urls.forEach(function(url) {
  driver.get(url).then(function() {
    driver.findElement(webdriver.By.css(exportSelector)).click();
    driver.sleep(3000).then(function() {
      driver
        .findElement(
          webdriver.By.css(
            "#beepboxEditorContainer > div > div.promptContainer > div > button:nth-child(5)"
          )
        )
        .click();
    });
  });
});
