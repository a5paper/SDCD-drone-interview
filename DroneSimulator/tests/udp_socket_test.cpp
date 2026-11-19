#include "network/udp_socket.h"

#include <gtest/gtest.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <thread>

namespace drone_simulator::network {
namespace {

constexpr uint16_t kTestPort = 5698;
// A lost datagram (netem loss on lo) fails the test instead of hanging it.
constexpr std::chrono::seconds kReceiveTimeout{1};
constexpr std::chrono::milliseconds kReceivePollInterval{1};

TEST(UdpSocketTest, ReceiveOnEmptySocketWouldBlock) {
  UdpSocket socket;
  ASSERT_TRUE(socket.Bind(kTestPort));
  ASSERT_TRUE(socket.SetNonBlocking());

  std::array<uint8_t, 64> buffer{};
  const ReceiveResult result = socket.Receive(buffer);

  EXPECT_EQ(result.status, ReceiveStatus::kWouldBlock);
}

TEST(UdpSocketTest, BindTwiceFailsWithErrorText) {
  UdpSocket first;
  ASSERT_TRUE(first.Bind(kTestPort));

  UdpSocket second;
  EXPECT_FALSE(second.Bind(kTestPort));
  const int error_code = LastSocketError();
  EXPECT_NE(error_code, 0);
  EXPECT_FALSE(ErrorText(error_code).empty());
}

TEST(UdpSocketTest, ConnectRejectsBadAddress) {
  UdpSocket socket;
  EXPECT_FALSE(socket.Connect("not-an-ip", kTestPort));
  EXPECT_NE(LastSocketError(), 0);
}

TEST(UdpSocketTest, LoopbackRoundTrip) {
  UdpSocket receiver;
  ASSERT_TRUE(receiver.Bind(kTestPort));
  ASSERT_TRUE(receiver.SetNonBlocking());
  UdpSocket sender;
  ASSERT_TRUE(sender.Connect("127.0.0.1", kTestPort));

  const std::array<uint8_t, 3> payload{1, 2, 3};
  const SendResult sent = sender.Send(payload);
  ASSERT_EQ(sent.status, SendStatus::kSent);

  std::array<uint8_t, 64> buffer{};
  const auto deadline = std::chrono::steady_clock::now() + kReceiveTimeout;
  ReceiveResult received = receiver.Receive(buffer);
  while (received.status == ReceiveStatus::kWouldBlock &&
         std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(kReceivePollInterval);
    received = receiver.Receive(buffer);
  }

  ASSERT_EQ(received.status, ReceiveStatus::kData);
  ASSERT_EQ(received.bytes, payload.size());
  EXPECT_EQ(buffer[0], 1);
  EXPECT_EQ(buffer[1], 2);
  EXPECT_EQ(buffer[2], 3);
}

}
}
