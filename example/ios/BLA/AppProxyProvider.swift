//
//  AppProxyProvider.swift
//  BLA
//
//  Created by TPS-User on 16/9/25.
//

import NetworkExtension
import local_push_connectivity_core

import UserNotifications

class AppProxyProvider: NEAppPushProvider {
    private var socket: ISocket? = nil
    override func start() {
        guard let _ = providerConfiguration?["host"] as? String else {
            return
        }
        socket = ISocket().register()
        socket?.connect()
        socket?.requestNotificationDebug(payload: "start")
    }
    
    override func stop(with reason: NEProviderStopReason, completionHandler: @escaping () -> Void) {
        socket?.disconnect()
//        socket = nil
        completionHandler()
    }
    
    override func handleTimerEvent() {
        socket?.reconnect()
    }
}
