var http = require("http");
var searchTwitter = require("./index");
var Crawler = require("crawler");
var fs = require("fs");
var beepBoxUrls = [];
searchTwitter.getTweets().then(function(data) {
  tweets = data;
  var c = new Crawler({
    maxConnections: 1,
    rateLimit: 1000,
    // This will be called for each crawled page
    callback: function(error, res, done) {
      if (error) {
        console.log(error);
      } else {
        var $ = res.$;
        var url = "";
        if (res.request.uri.href.includes("#")) {
          beepBoxUrls.push(res.request.uri.href);
        }
      }
      done();
    }
  });

  // Queue a list of URLs
  c.queue(tweets);
  c.on("drain", function() {
    beepBoxUrls = beepBoxUrls.filter(function(elem, pos) {
      return beepBoxUrls.indexOf(elem) == pos;
    });
    fs.readFile("beepBoxUrls.json", "utf8", function readFileCallback(
      err,
      data
    ) {
      if (err) {
        console.log("cant write to file", err);
      } else {
        if (data.length < 1) {
          json = JSON.stringify(beepBoxUrls);
        } else {
          var present = JSON.parse(data);
          json = JSON.stringify(beepBoxUrls.concat(present));
        }
        fs.writeFile("beepBoxUrls.json", json);
      }
    });
  });
});
