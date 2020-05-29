/**
 * Copyright 2013 dc-square GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @author: Christoph Schäbel
 */

var websocketclient = {
    'client': null,
    'lastMessageId': 1,
    'lastSubId': 1,
    'subscriptions': [],
    'messages': [],
    'connected': false,
	'stringValue': "",
	'stringValue2': "",

    'connect': function () {

        //var host = $('#urlInput').val();
		//var host = "192.168.1.15";
		var host = "31.35.174.113";
        //var port = parseInt($('#portInput').val(), 10);
		var port = 9001
        //var clientId = $('#clientIdInput').val();
		var clientId = "DCND";
        //var username = $('#userInput').val();
		var username = "dyd";
        //var password = $('#pwInput').val();
		var password = "doyourdream";
		//var keepAlive = parseInt($('#keepAliveInput').val());
        var keepAlive = 60;
		//var cleanSession = $('#cleanSessionInput').is(':checked');
        var cleanSession = true;
		//var lwTopic = $('#lwTopicInput').val();
        var lwTopic = "";
		//var lwQos = parseInt($('#lwQosInput').val());
        var lwQos = 0;
		//var lwRetain = $('#LWRInput').is(':checked');
        var lwRetain = false;
		//var lwMessage = $('#LWMInput').val();
        var lwMessage = "";
		//var ssl = $('#sslInput').is(':checked');
        var ssl = false;

        this.client = new Messaging.Client(host, port, clientId);
        this.client.onConnectionLost = this.onConnectionLost;
        this.client.onMessageArrived = this.onMessageArrived;

        var options = {
            timeout: 3,
            keepAliveInterval: keepAlive,
            cleanSession: cleanSession,
            useSSL: ssl,
            onSuccess: this.onConnect,
            onFailure: this.onFail
        };

        if (username.length > 0) {
            options.userName = username;
        }
        if (password.length > 0) {
            options.password = password;
        }
        if (lwTopic.length > 0) {
            var willmsg = new Messaging.Message(lwMessage);
            willmsg.qos = lwQos;
            willmsg.destinationName = lwTopic;
            willmsg.retained = lwRetain;
            options.willMessage = willmsg;
        }

        this.client.connect(options);
    },

    'onConnect': function () {
        websocketclient.connected = true;
        console.log("connected");
        var body = $('body').addClass('connected').removeClass('notconnected').removeClass('connectionbroke');

        websocketclient.render.hide('conni');
        websocketclient.render.show('publish');
        websocketclient.render.show('sub');
        websocketclient.render.show('messages');		
    },

    'onFail': function (message) {
        websocketclient.connected = false;
        console.log("error: " + message.errorMessage);
        websocketclient.render.showError('Connect failed: ' + message.errorMessage);
    },

    'onConnectionLost': function (responseObject) {
        websocketclient.connected = false;
        if (responseObject.errorCode !== 0) {
            console.log("onConnectionLost:" + responseObject.errorMessage);
        }
        $('body.connected').removeClass('connected').addClass('notconnected').addClass('connectionbroke');
        websocketclient.render.show('conni');
        websocketclient.render.hide('publish');
        websocketclient.render.hide('sub');
        websocketclient.render.hide('messages');

        //Cleanup messages
        websocketclient.messages = [];
        websocketclient.render.clearMessages();

        //Cleanup subscriptions
        websocketclient.subscriptions = [];
        websocketclient.render.clearSubscriptions();
    },

    'onMessageArrived': function (message) {
//        console.log("onMessageArrived:" + message.payloadString + " qos: " + message.qos);

        var subscription = websocketclient.getSubscriptionForTopic(message.destinationName);

        var messageObj = {
            'topic': message.destinationName,
            'retained': message.retained,
            'qos': message.qos,
            'payload': message.payloadString,
            'timestamp': moment(),
            'subscriptionId': subscription.id,
            'color': websocketclient.getColorForSubscription(subscription.id)

        };
        console.log(messageObj);
        messageObj.id = websocketclient.render.message(messageObj);
        websocketclient.messages.push(messageObj);
    },

    'disconnect': function () {
        this.client.disconnect();
    },

    'publishMessage': function (payload) {
		
		if(payload.length > 0)
		{
			if(payload.length <= 207)
			{
				websocketclient.moveProgressBarMsg(0);
				
				if (!websocketclient.connected) {
					/*websocketclient.render.showError("Not connected");
					return false;*/
					websocketclient.connect();
				}
				
				var startMessage = new Messaging.Message("START_TRANSFER_MSG\n");
				startMessage.destinationName = "/topic/dcnd/msg";
				startMessage.qos = 0;
				startMessage.retained = false;
				
				console.log("Message : " + payload);
				var message = new Messaging.Message(payload+"\n");
				message.destinationName = "/topic/dcnd/msg";
				message.qos = 0;
				message.retained = false;
				
				var stopMessage = new Messaging.Message("STOP_TRANSFER_MSG\n");
				stopMessage.destinationName = "/topic/dcnd/msg";
				stopMessage.qos = 0;
				stopMessage.retained = false;
				
				//this.client.send(message);
				
				// websocketclient.moveProgressBarMsg();
				
				if(!websocketclient.connected)
				{
					setTimeout(() => {
						websocketclient.moveProgressBarMsg(35);
						this.client.send(startMessage);
						setTimeout(() => {
							websocketclient.moveProgressBarMsg(70);
							this.client.send(message);
							setTimeout(() => {
								websocketclient.moveProgressBarMsg(100);
								this.client.send(stopMessage);
							}, 1000);
						}, 1000);
					}, 1000);
				}
				else
				{
					websocketclient.moveProgressBarMsg(35);
					this.client.send(startMessage);
					setTimeout(() => {
						websocketclient.moveProgressBarMsg(70);
						this.client.send(message);
						setTimeout(() => {
							websocketclient.moveProgressBarMsg(100);
							this.client.send(stopMessage);
						}, 1000);
					}, 1000);
				}
			}else{
				websocketclient.render.showError("Your message is too long !\nThe number of characters need to be lower than 207");
			}
		}else{
			websocketclient.render.showError("Your message is empty");
		}
    },
	
	'moveProgressBarMsg': function (width) {
		var progressBarMsg = document.getElementById("barMsg");

		progressBarMsg.style.width = width  + "%";
		progressBarMsg.innerHTML = width  + "%";
	},

	'sleep': function (milliseconds) {
		const date = Date.now();
		let currentDate = null;
		do {
			currentDate = Date.now();
		} while (currentDate - date < milliseconds);
	},

    'publishImage': function () {
		//websocketclient.render.showError("Function not implemented");
		//return false;
		
		if (!websocketclient.connected) {
            /*websocketclient.render.showError("Not connected");
            return false;*/
			websocketclient.connect();
        }
		
		var startImg = new Messaging.Message("START_TRANSFER_IMG\n");
		startImg.destinationName = "/topic/dcnd/msg";
        startImg.qos = 0;
        startImg.retained = false;

		var output_string = "", count = 1;
		var code = "";
		
		images.each(function(image) {
			code = imageToString(image);
			//var comment = image.glyph ? ("// '" + image.glyph + "', " + image.canvas.width+"x"+image.canvas.height+"px\n") : "";
			//if(image.img != images.first().img) comment = "\n" + comment;
			//code = comment + code;
			output_string += code;
		});
		
		//console.log("output_string = " + output_string);
		
		var imagesMsg = new Messaging.Message(output_string);
		imagesMsg.destinationName = "/topic/dcnd/msg";
        imagesMsg.qos = 0;
        imagesMsg.retained = false;
		
		/*var i, output_index;
		for (i = 0; i < output_string.length; i++) { 
			if(i )
		}*/
		
		var stopImg = new Messaging.Message("STOP_TRANSFER_IMG\n");
		stopImg.destinationName = "/topic/dcnd/msg";
        stopImg.qos = 0;
        stopImg.retained = false;
		
		if(!websocketclient.connected)
		{
			setTimeout(() => {
				websocketclient.moveProgressBarImg(35);
				console.log("START_TRANSFER_IMG");
				this.client.send(startImg);
				setTimeout(() => {
					console.log("imagesMsg");
					
					var i, output_index, subMsg, temp;
					var startIndex, lengh;
					
					temp = 35;
					lengh = 192;
					startIndex = 0;
					
					console.log("Size of message = " + output_string.length);
					console.log("Size of each message = " + lengh);
					console.log("Full message = " + websocketclient.stringValue2);
					
					for (i = 0; i < (output_string.length/lengh); i++) {
					//for (i = 0; i < 3; i++) {
						
						console.log("ProgressBarImg = " + temp);
						
						temp = temp + (50/(output_string.length/lengh));
						websocketclient.moveProgressBarImg(50);
						startIndex = lengh*i;
						
						console.log("start index = " + startIndex);
						
						var subMsg = output_string.substr(startIndex, lengh);
						var subMsg2 = websocketclient.stringValue.substr(startIndex*3, 192);
						
						console.log("Raw message = " + subMsg);
						console.log("Raw modify message = " + subMsg2);
						
						var imgMsg = new Messaging.Message(subMsg+"\n");
						imgMsg.destinationName = "/topic/dcnd/msg";
						imgMsg.qos = 0;
						imgMsg.retained = false;
						
						this.client.send(imgMsg);
						
						websocketclient.sleep(250);
					}
					
					/*startIndex = 900*(i+1);
					
					console.log("start index = " + startIndex);
					
					var subMsg = output_string.substr(startIndex, output_string.length-startIndex);
						
					console.log(subMsg);
					
					var imgMsg = new Messaging.Message(subMsg+"\n");
					imgMsg.destinationName = "/topic/dcnd/msg";
					imgMsg.qos = 0;
					imgMsg.retained = false;
					
					this.client.send(imgMsg);*/
					
					//this.client.send(imagesMsg);
					setTimeout(() => {
						websocketclient.moveProgressBarImg(100);
						console.log("STOP_TRANSFER_IMG");
						this.client.send(stopImg);
					}, 1000);
				}, 1000);
			}, 1000);
		}
		else
		{
			websocketclient.moveProgressBarImg(35);
			console.log("START_TRANSFER_IMG");
			this.client.send(startImg);

			setTimeout(() => {
				console.log("imagesMsg");
				
				var i, output_index, subMsg, temp;
				var startIndex, lengh;
				
				lengh = 192;
				temp = 35;
				startIndex = 0;
				
				console.log("Size of message = " + output_string.length);
				console.log("Size of each message = " + lengh);
				console.log("Full message = " + websocketclient.stringValue2);
				
				for (i = 0; i < (output_string.length/lengh); i++) {
				//for (i = 0; i < 3; i++) {
					
					console.log("ProgressBarImg = " + temp);
					
					temp = temp + (50/(output_string.length/lengh));
					websocketclient.moveProgressBarImg(50);
					startIndex = lengh*i;
					
					console.log("start index = " + startIndex);
					
					var subMsg = output_string.substr(startIndex, lengh);
					var subMsg2 = websocketclient.stringValue.substr(startIndex*3, 192);
					
					console.log("Raw message = " + subMsg);
					console.log("Raw modify message = " + subMsg2);
					
					var imgMsg = new Messaging.Message(subMsg+"\n");
					imgMsg.destinationName = "/topic/dcnd/msg";
					imgMsg.qos = 0;
					imgMsg.retained = false;
					
					this.client.send(imgMsg);
					
					websocketclient.sleep(250);
				}
				
				//this.client.send(imagesMsg);
				setTimeout(() => {
					websocketclient.moveProgressBarImg(100);
					console.log("STOP_TRANSFER_IMG");
					this.client.send(stopImg);
				}, 1000);
			}, 1000);
		}
    },
	
	'moveProgressBarImg': function (width) {
		var progressBarImg = document.getElementById("barImg");

		progressBarImg.style.width = width  + "%";
		progressBarImg.innerHTML = width  + "%";
	},

    'subscribe': function (topic, qosNr, color) {

        if (!websocketclient.connected) {
            websocketclient.render.showError("Not connected");
            return false;
        }

        if (topic.length < 1) {
            websocketclient.render.showError("Topic cannot be empty");
            return false;
        }

        if (_.find(this.subscriptions, { 'topic': topic })) {
            websocketclient.render.showError('You are already subscribed to this topic');
            return false;
        }

        this.client.subscribe(topic, {qos: qosNr});
        if (color.length < 1) {
            color = '999999';
        }

        var subscription = {'topic': topic, 'qos': qosNr, 'color': color};
        subscription.id = websocketclient.render.subscription(subscription);
        this.subscriptions.push(subscription);
        return true;
    },

    'unsubscribe': function (id) {
        var subs = _.find(websocketclient.subscriptions, {'id': id});
        this.client.unsubscribe(subs.topic);
        websocketclient.subscriptions = _.filter(websocketclient.subscriptions, function (item) {
            return item.id != id;
        });

        websocketclient.render.removeSubscriptionsMessages(id);
    },

    'deleteSubscription': function (id) {
        var elem = $("#sub" + id);

        if (confirm('Are you sure ?')) {
            elem.remove();
            this.unsubscribe(id);
        }
    },

    'getRandomColor': function () {
        var r = (Math.round(Math.random() * 255)).toString(16);
        var g = (Math.round(Math.random() * 255)).toString(16);
        var b = (Math.round(Math.random() * 255)).toString(16);
        return r + g + b;
    },

    'getSubscriptionForTopic': function (topic) {
        var i;
        for (i = 0; i < this.subscriptions.length; i++) {
            if (this.compareTopics(topic, this.subscriptions[i].topic)) {
                return this.subscriptions[i];
            }
        }
        return false;
    },

    'getColorForPublishTopic': function (topic) {
        var id = this.getSubscriptionForTopic(topic);
        return this.getColorForSubscription(id);
    },

    'getColorForSubscription': function (id) {
        try {
            if (!id) {
                return '99999';
            }

            var sub = _.find(this.subscriptions, { 'id': id });
            if (!sub) {
                return '999999';
            } else {
                return sub.color;
            }
        } catch (e) {
            return '999999';
        }
    },

    'compareTopics': function (topic, subTopic) {
        var pattern = subTopic.replace("+", "(.*?)").replace("#", "(.*)");
        var regex = new RegExp("^" + pattern + "$");
        return regex.test(topic);
    },

    'render': {

        'showError': function (message) {
            alert(message);
        },
        'messages': function () {

            websocketclient.render.clearMessages();
            _.forEach(websocketclient.messages, function (message) {
                message.id = websocketclient.render.message(message);
            });

        },
        'message': function (message) {

            var largest = websocketclient.lastMessageId++;

            var html = '<li class="messLine id="' + largest + '">' +
                '   <div class="row large-12 mess' + largest + '" style="border-left: solid 10px #' + message.color + '; ">' +
                '       <div class="large-12 columns messageText">' +
                '           <div class="large-3 columns date">' + message.timestamp.format("YYYY-MM-DD HH:mm:ss") + '</div>' +
                '           <div class="large-5 columns topicM truncate" id="topicM' + largest + '" title="' + Encoder.htmlEncode(message.topic, 0) + '">Topic: ' + Encoder.htmlEncode(message.topic) + '</div>' +
                '           <div class="large-2 columns qos">Qos: ' + message.qos + '</div>' +
                '           <div class="large-2 columns retain">';
            if (message.retained) {
                html += 'Retained';
            }
            html += '           </div>' +
                '           <div class="large-12 columns message break-words">' + Encoder.htmlEncode(message.payload) + '</div>' +
                '       </div>' +
                '   </div>' +
                '</li>';
            $("#messEdit").prepend(html);
            return largest;
        },

        'subscriptions': function () {
            websocketclient.render.clearSubscriptions();
            _.forEach(websocketclient.subscriptions, function (subs) {
                subs.id = websocketclient.render.subscription(subs);
            });
        },

        'subscription': function (subscription) {
            var largest = websocketclient.lastSubId++;
            $("#innerEdit").append(
                '<li class="subLine" id="sub' + largest + '">' +
                    '   <div class="row large-12 subs' + largest + '" style="border-left: solid 10px #' + subscription.color + '; background-color: #ffffff">' +
                    '       <div class="large-12 columns subText">' +
                    '           <div class="large-1 columns right closer">' +
                    '              <a href="#" onclick="websocketclient.deleteSubscription(' + largest + '); return false;">x</a>' +
                    '           </div>' +
                    '           <div class="qos">Qos: ' + subscription.qos + '</div>' +
                    '           <div class="topic truncate" id="topic' + largest + '" title="' + Encoder.htmlEncode(subscription.topic, 0) + '">' + Encoder.htmlEncode(subscription.topic) + '</div>' +
                    '       </div>' +
                    '   </div>' +
                    '</li>');
            return largest;
        },

        'toggleAll': function () {
            websocketclient.render.toggle('conni');
            websocketclient.render.toggle('publish');
            websocketclient.render.toggle('messages');
            websocketclient.render.toggle('sub');
        },

        'toggle': function (name) {
            $('.' + name + 'Arrow').toggleClass("closed");
            $('.' + name + 'Top').toggleClass("closed");
            var elem = $('#' + name + 'Main');
            elem.slideToggle();
        },

        'hide': function (name) {
            $('.' + name + 'Arrow').addClass("closed");
            $('.' + name + 'Top').addClass("closed");
            var elem = $('#' + name + 'Main');
            elem.slideUp();
        },

        'show': function (name) {
            $('.' + name + 'Arrow').removeClass("closed");
            $('.' + name + 'Top').removeClass("closed");
            var elem = $('#' + name + 'Main');
            elem.slideDown();
        },

        'removeSubscriptionsMessages': function (id) {
            websocketclient.messages = _.filter(websocketclient.messages, function (item) {
                return item.subscriptionId != id;
            });
            websocketclient.render.messages();
        },

        'clearMessages': function () {
            $("#messEdit").empty();
        },

        'clearSubscriptions': function () {
            $("#innerEdit").empty();
        }
    }
};
