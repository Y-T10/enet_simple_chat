#pragma once

#include<enet/enet.h>

/**
 * パケット送信関数
 * @param det 送信先
 * @param channel 送信チャンネル
 * @param data データのアドレス
 * @param data_size データのバイトサイズ
 * @param is_reliable 送信を安定させるかのフラグ
 * @return 送信の成否
 */
const bool Send_ENet_Packet(ENetPeer* dst, const size_t channel, const void* data, const size_t data_size, const bool is_reliable);

/**
 * パケット一斉送信関数
 * @param server サーバ
 * @param channel 送信チャンネル
 * @param data データのアドレス
 * @param data_size データのバイトサイズ
 * @param is_reliable 送信を安定させるかのフラグ
 * @return 送信の成否
 */
const bool Broadcast_ENet_Packet(ENetHost* server, const size_t channel, const void* data, const size_t data_size, const bool is_reliable);