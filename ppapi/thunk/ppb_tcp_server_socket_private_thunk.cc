// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ppapi/c/pp_errors.h"
#include "ppapi/c/private/ppb_tcp_server_socket_private.h"
#include "ppapi/thunk/enter.h"
#include "ppapi/thunk/ppb_tcp_server_socket_private_api.h"
#include "ppapi/thunk/resource_creation_api.h"
#include "ppapi/thunk/thunk.h"

namespace ppapi {
namespace thunk {

namespace {

typedef EnterResource<PPB_TCPServerSocket_Private_API> EnterTCPServer;

PP_Resource Create(PP_Instance instance) {
  EnterFunction<ResourceCreationAPI> enter(instance, true);
  if (enter.failed())
    return 0;
  return enter.functions()->CreateTCPServerSocketPrivate(instance);
}

PP_Bool IsTCPServerSocket(PP_Resource resource) {
  EnterTCPServer enter(resource, false);
  return PP_FromBool(enter.succeeded());
}

int32_t Listen(PP_Resource tcp_server_socket,
               const PP_NetAddress_Private* addr,
               int32_t backlog,
               PP_CompletionCallback callback) {
  EnterTCPServer enter(tcp_server_socket, callback, true);
  if (enter.failed())
    return enter.retval();
  return enter.SetResult(enter.object()->Listen(addr, backlog, callback));
}

int32_t Accept(PP_Resource tcp_server_socket,
               PP_Resource* tcp_socket,
               PP_CompletionCallback callback) {
  EnterTCPServer enter(tcp_server_socket, callback, true);
  if (enter.failed())
    return enter.retval();
  return enter.SetResult(enter.object()->Accept(tcp_socket, callback));
}

void StopListening(PP_Resource tcp_server_socket) {
  EnterTCPServer enter(tcp_server_socket, true);
  if (enter.succeeded())
    enter.object()->StopListening();
}

const PPB_TCPServerSocket_Private g_ppb_tcp_server_socket_thunk = {
  Create,
  IsTCPServerSocket,
  Listen,
  Accept,
  StopListening
};

}  // namespace

const PPB_TCPServerSocket_Private* GetPPB_TCPServerSocket_Private_0_1_Thunk() {
  return &g_ppb_tcp_server_socket_thunk;
}

}  // namespace thunk
}  // namespace ppapi
