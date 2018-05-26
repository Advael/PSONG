

module.exports = {
  getTweets: function() {
    var tweets = [];
    var Twit = require("twit");
    var fs = require("fs");
    var f = "./crawl.txt";
    var T = new Twit({
      consumer_key: "IKh586IL802DfWyHVHZmngRYu",
      consumer_secret: "bN2sG2uVQ4tVlxQTGqCI2Z8jSktQbHsobijrLM2ENifCmrH0YD",
      access_token: "250444181-BtS87DX5wyxGEXXu3uJ331YFxYaci7LTZ9ppJSC8",
      access_token_secret: "FhaNi0rogFR6ZAYASRqGX7cABiu3Ief18L5uGYK51MVtM",
      timeout_ms: 60 * 1000 // optional HTTP request timeout to apply to all requests.
    });
    return new Promise(function(resolve, reject) {
      T.get(
        "search/tweets",
        {
          q: "beepbox",
          count: 1000,
          tweet_mode: "extended",
          include_entities: true
        },
        function(err, data, response) {
          for (var i = 0; i < Object.keys(data.statuses).length; i++) {
            var tweetUrls = data.statuses[i].entities.urls[0];
            if (tweetUrls !== undefined) {
              tweets.push(tweetUrls.url);
            }
          }
          resolve(tweets);
        }
      );
    });
  }
};
