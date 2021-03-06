// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
This component extension test does the following:

1. Creates an abc and log file on the local file system with some random text.
2. Finds a registered task (file item context menu) for abc file and invokes it
   with url of the test file.
3. Listens for a message from context menu handler and makes sure its payload
   matches the random text from the test file.
*/

var cleanupError = 'Got unexpected error while cleaning up test directory.';

// Class specified by the client runnig the TestRunner.
// |expectedTasks| should contain list of actions defined for abc files defined
//     by filesystem_handler part of the test.
// |fileVerifierFunction| method that will verify test results received from the
//     filesystem_handler part of the test.
//     The method will be passed received fileEntry object, original file
//     content, file content received from filesystem_handler and callback
//     function that will expect error object as its argument (or undefined on
//     success).
var TestExpectations = function(expectedTasks, fileVerifierFunction) {
  this.fileText_ = undefined;
  this.file_ = undefined;
  this.expectedTasks_ = expectedTasks;
  this.fileVerifierFunction_ = fileVerifierFunction;
};

// This has to be called before verifyHandlerRequest.
TestExpectations.prototype.setFileAndFileText = function(file, fileText) {
  this.file_ = file;
  this.fileText_ = fileText;
};

TestExpectations.prototype.verifyHandlerRequest = function(request, callback) {
  if (!request) {
    callback({message: "Request from handler not defined."});
    return;
  }

  if (!request.fileContent) {
    var error = request.error || {message: "Undefined error."};
    callback(error);
    return;
  }

  if (!this.file_ || !this.fileText_ || !this.fileVerifierFunction_) {
    callback({message: "Test expectations not set properly."});
    return;
  }

  this.fileVerifierFunction_(this.file_, this.fileText_, request.fileContent,
                             callback);
};

// Verifies that list of tasks |tasks| contains tasks specified in
// expectedTasks_. |successCallback| expects to be passed |tasks|.
// |errorCallback| expects error object.
TestExpectations.prototype.verifyTasks = function(tasks,
                                                  successCallback,
                                                  errorCallback) {
  if (tasks.length != Object.keys(this.expectedTasks_).length) {
    errorCallback({message: 'Wrong number of tasks found.'});
    return;
  }

  for (var i = 0; i < tasks.length; ++i) {
    var taskName = /^.*[|](\w+)$/.exec(tasks[i].taskId)[1];
    var patterns = tasks[i].patterns;
    var expectedPatterns = this.expectedTasks_[taskName];
    if (!expectedPatterns) {
      errorCallback({message: 'Wrong task from getFileTasks(): ' + task_name});
      return;
    }
    patterns = patterns.sort();
    expectedPatterns = expectedPatterns.sort();
    for (var j = 0; j < patterns.length; ++j) {
      if (patterns[j] != expectedPatterns[j]) {
        errorCallback({message: 'Wrong patterns set for task ' +
                                taskName + '. ' +
                                'Got: ' + patterns +
                                ' expected: ' + expectedPatterns});
        return;
      }
    }
  }
  successCallback(tasks);
};

// Class that is in charge for running the test.
var TestRunner = function(expectations) {
  this.expectations_ = expectations;
  this.fileCreator_ = new TestFileCreator("tmp", true /* shouldRandomize */);
  this.listener_ = this.onHandlerRequest_.bind(this);
};

// Starts the test.
TestRunner.prototype.runTest = function() {
  // Get local FS, create dir with a file in it.
  console.log('Requesting local file system...');
  chrome.extension.onRequestExternal.addListener(this.listener_);
  chrome.fileBrowserPrivate.requestLocalFileSystem(
      this.onFileSystemFetched_.bind(this));
};

TestRunner.prototype.onFileSystemFetched_ = function(fs) {
  if (!fs) {
    this.errorCallback_(chrome.extensions.lastError);
    return;
  }

  this.fileCreator_.init(fs, this.onFileCreatorInit_.bind(this),
                             this.errorCallback_.bind(this));
};

TestRunner.prototype.onFileCreatorInit_ = function() {
  var self = this;
  this.fileCreator_.createFile('.log',
      function(file, text) {
        self.fileCreator_.createFile('.aBc',
            self.onFileCreated_.bind(self),
            self.errorCallback_.bind(self));
      },
      this.errorCallback_.bind(this));
};

TestRunner.prototype.onFileCreated_ = function(file, text) {
  // Start
  console.log('Get registered tasks now...');
  this.expectations_.setFileAndFileText(file, text);
  var fileUrl = file.toURL();

  chrome.fileBrowserPrivate.getFileTasks([fileUrl],
                                         this.onGetTasks_.bind(this, fileUrl));
};

TestRunner.prototype.onGetTasks_ = function(fileUrl, tasks) {
  console.log('Tasks: ');
  console.log(tasks);
  if (!tasks || !tasks.length) {
    this.errorCallback_({message: 'No tasks registered'});
    return;
  }

  console.log('DONE fetching ' + tasks.length + ' tasks');

  this.expectations_.verifyTasks(tasks,
                                 this.onTasksVerified_.bind(this, fileUrl),
                                 this.errorCallback_.bind(this));
}

TestRunner.prototype.onTasksVerified_ = function(fileUrl, tasks) {
  chrome.fileBrowserPrivate.executeTask(tasks[0].taskId, [fileUrl]);
};

TestRunner.prototype.errorCallback_ = function(error) {
  var msg = '';
  if (!error.code) {
    msg = error.message;
  } else {
    switch (error.code) {
      case FileError.QUOTA_EXCEEDED_ERR:
        msg = 'QUOTA_EXCEEDED_ERR';
        break;
      case FileError.NOT_FOUND_ERR:
        msg = 'NOT_FOUND_ERR';
        break;
      case FileError.SECURITY_ERR:
        msg = 'SECURITY_ERR';
        break;
      case FileError.INVALID_MODIFICATION_ERR:
        msg = 'INVALID_MODIFICATION_ERR';
        break;
      case FileError.INVALID_STATE_ERR:
        msg = 'INVALID_STATE_ERR';
        break;
      default:
        msg = 'Unknown Error';
        break;
    };
  }

  this.fileCreator_.cleanupAndEndTest(
      this.reportFail_.bind(this, 'Got unexpected error: ' + msg),
      this.reportFail_.bind(this, 'Got unexpected error: ' + msg));
};

TestRunner.prototype.reportSuccess_ = function(entry) {
  chrome.test.succeed();
};

TestRunner.prototype.reportFail_ = function(message) {
  chrome.test.fail(message);
};

// Listens for the request from the filesystem_handler extension. When the
// event is received, it verifies it and stops listening for further events.
TestRunner.prototype.onHandlerRequest_ =
    function(request, sender, sendResponse) {
  this.expectations_.verifyHandlerRequest(
      request,
      this.verifyRequestCallback_.bind(this, sendResponse));
  chrome.extension.onRequestExternal.removeListener(this.listener_);
};

TestRunner.prototype.verifyRequestCallback_ = function(sendResponse, error) {
  if (!error) {
    sendResponse({success: true});
    this.fileCreator_.cleanupAndEndTest(this.reportSuccess_.bind(this),
                                        this.reportFail_.bind(this,
                                                              cleanupError));
  } else {
    sendResponse({success: false});
    this.errorCallback_(error);
  }
};
