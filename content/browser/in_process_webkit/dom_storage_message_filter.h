// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_IN_PROCESS_WEBKIT_DOM_STORAGE_MESSAGE_FILTER_H_
#define CONTENT_BROWSER_IN_PROCESS_WEBKIT_DOM_STORAGE_MESSAGE_FILTER_H_
#pragma once

#include "base/memory/ref_counted.h"
#include "base/message_loop_helpers.h"
#include "base/process.h"
#include "content/browser/in_process_webkit/dom_storage_area.h"
#include "content/common/dom_storage_common.h"
#include "content/public/browser/browser_message_filter.h"
#include "webkit/dom_storage/dom_storage_types.h"

#ifdef ENABLE_NEW_DOM_STORAGE_BACKEND
// This class is replaced by a new implementation in
#include "content/browser/dom_storage/dom_storage_message_filter_new.h"
#else

class DOMStorageContextImpl;
class GURL;
struct DOMStorageMsg_Event_Params;

// This class handles the logistics of DOM Storage within the browser process.
// It mostly ferries information between IPCs and the WebKit implementations,
// but it also handles some special cases like when renderer processes die.
class DOMStorageMessageFilter : public content::BrowserMessageFilter {
 public:
  // Only call the constructor from the UI thread.
  DOMStorageMessageFilter(int process_id,
                          DOMStorageContextImpl* dom_storage_context);

  // content::BrowserMessageFilter implementation
  virtual void OnChannelConnected(int32 peer_pid) OVERRIDE;
  virtual void OverrideThreadForMessage(
      const IPC::Message& message,
      content::BrowserThread::ID* thread) OVERRIDE;
  virtual bool OnMessageReceived(const IPC::Message& message,
                                 bool* message_was_ok) OVERRIDE;
  virtual void OnDestruct() const OVERRIDE;

  // Only call on the WebKit thread.
  static void DispatchStorageEvent(const NullableString16& key,
      const NullableString16& old_value, const NullableString16& new_value,
      const string16& origin, const GURL& url, bool is_local_storage);

 private:
  friend class content::BrowserThread;
  friend class base::DeleteHelper<DOMStorageMessageFilter>;
  virtual ~DOMStorageMessageFilter();

  // Message Handlers.
  void OnOpenStorageArea(int64 namespace_id, const string16& origin,
                         int64* storage_area_id);
  void OnCloseStorageArea(int64 storage_area_id);
  void OnLength(int64 storage_area_id, unsigned* length);
  void OnKey(int64 storage_area_id, unsigned index, NullableString16* key);
  void OnGetItem(int64 storage_area_id, const string16& key,
                 NullableString16* value);
  void OnSetItem(int64 storage_area_id, const string16& key,
                 const string16& value, const GURL& url,
                 WebKit::WebStorageArea::Result* result,
                 NullableString16* old_value);
  void OnRemoveItem(int64 storage_area_id, const string16& key,
                    const GURL& url, NullableString16* old_value);
  void OnClear(int64 storage_area_id, const GURL& url, bool* something_cleared);

  // Only call on the IO thread.
  void OnStorageEvent(const DOMStorageMsg_Event_Params& params);

  // A shortcut for accessing our context.
  DOMStorageContextImpl* Context() { return dom_storage_context_; }

  // Use whenever there's a chance OnStorageEvent will be called.
  class ScopedStorageEventContext {
   public:
    ScopedStorageEventContext(
        DOMStorageMessageFilter* dispatcher_message_filter,
        const GURL* url);
    ~ScopedStorageEventContext();
  };

  // Only access on the WebKit thread!  Used for storage events.
  static DOMStorageMessageFilter* storage_event_message_filter;
  static const GURL* storage_event_url_;

  scoped_refptr<DOMStorageContextImpl> dom_storage_context_;

  // Used to dispatch messages to the correct view host.
  int process_id_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(DOMStorageMessageFilter);
};

#endif  // ENABLE_NEW_DOM_STORAGE_BACKEND
#endif  // CONTENT_BROWSER_IN_PROCESS_WEBKIT_DOM_STORAGE_MESSAGE_FILTER_H_

