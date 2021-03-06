// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_PEPPER_PEPPER_PROXY_CHANNEL_DELEGATE_IMPL_H_
#define CONTENT_RENDERER_PEPPER_PEPPER_PROXY_CHANNEL_DELEGATE_IMPL_H_

#include "base/compiler_specific.h"
#include "ppapi/proxy/proxy_channel.h"

class PepperProxyChannelDelegateImpl
    : public ppapi::proxy::ProxyChannel::Delegate {
 public:
  virtual ~PepperProxyChannelDelegateImpl();

  // ProxyChannel::Delegate implementation.
  virtual base::MessageLoopProxy* GetIPCMessageLoop() OVERRIDE;
  virtual base::WaitableEvent* GetShutdownEvent() OVERRIDE;
};

#endif  // CONTENT_RENDERER_PEPPER_PEPPER_PROXY_CHANNEL_DELEGATE_IMPL_H_
