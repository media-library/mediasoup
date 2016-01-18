#ifndef MS_RTC_TRANSPORT_H
#define MS_RTC_TRANSPORT_H

#include "common.h"
#include "RTC/UDPSocket.h"
#include "RTC/TCPServer.h"
#include "RTC/TCPConnection.h"
#include "RTC/IceCandidate.h"
#include "RTC/ICEServer.h"
#include "RTC/STUNMessage.h"
#include "RTC/TransportTuple.h"
#include "RTC/DTLSTransport.h"
#include "RTC/RTPPacket.h"
#include "RTC/RTCPPacket.h"
#include "RTC/RtpListener.h"
#include "Channel/Request.h"
#include "Channel/Notifier.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <json/json.h>

namespace RTC
{
	class Transport :
		public RTC::UDPSocket::Listener,
		public RTC::TCPServer::Listener,
		public RTC::TCPConnection::Listener,
		public RTC::ICEServer::Listener,
		public RTC::DTLSTransport::Listener,
		// Transport is also a RtpListener.
		public RTC::RtpListener
	{
	public:
		class Listener
		{
		public:
			virtual void onTransportClosed(RTC::Transport* transport) = 0;
		};

	public:
		Transport(Listener* listener, Channel::Notifier* notifier, uint32_t transportId, Json::Value& data, Transport* rtpTransport = nullptr);
		virtual ~Transport();

		void Close();
		Json::Value toJson();
		void HandleRequest(Channel::Request* request);
		std::string& GetIceUsernameFragment();
		std::string& GetIcePassword();
		Transport* CreateAssociatedTransport(uint32_t transportId);

	private:
		void MayRunDTLSTransport();

	/* Private methods to unify UDP and TCP behavior. */
	private:
		void onPacketRecv(RTC::TransportTuple* tuple, const uint8_t* data, size_t len);
		void onSTUNDataRecv(RTC::TransportTuple* tuple, const uint8_t* data, size_t len);
		void onDTLSDataRecv(RTC::TransportTuple* tuple, const uint8_t* data, size_t len);
		void onRTPDataRecv(RTC::TransportTuple* tuple, const uint8_t* data, size_t len);
		void onRTCPDataRecv(RTC::TransportTuple* tuple, const uint8_t* data, size_t len);

	/* Pure virtual methods inherited from RTC::UDPSocket::Listener. */
	public:
		virtual void onPacketRecv(RTC::UDPSocket *socket, const uint8_t* data, size_t len, const struct sockaddr* remote_addr) override;

	/* Pure virtual methods inherited from RTC::TCPServer::Listener. */
	public:
		virtual void onRTCTCPConnectionClosed(RTC::TCPServer* tcpServer, RTC::TCPConnection* connection, bool is_closed_by_peer) override;

	/* Pure virtual methods inherited from RTC::TCPConnection::Listener. */
	public:
		virtual void onPacketRecv(RTC::TCPConnection *connection, const uint8_t* data, size_t len) override;

	/* Pure virtual methods inherited from RTC::ICEServer::Listener. */
	public:
		virtual void onOutgoingSTUNMessage(RTC::ICEServer* iceServer, RTC::STUNMessage* msg, RTC::TransportTuple* tuple) override;
		virtual void onICESelectedTuple(ICEServer* iceServer, RTC::TransportTuple* tuple) override;
		virtual void onICEConnected(ICEServer* iceServer) override;
		virtual void onICECompleted(ICEServer* iceServer) override;
		virtual void onICEDisconnected(ICEServer* iceServer) override;

	/* Pure virtual methods inherited from RTC::DTLSTransport::Listener. */
	public:
		virtual void onDTLSConnecting(DTLSTransport* dtlsTransport) override;
		virtual void onDTLSConnected(DTLSTransport* dtlsTransport, RTC::SRTPSession::SRTPProfile srtp_profile, uint8_t* srtp_local_key, size_t srtp_local_key_len, uint8_t* srtp_remote_key, size_t srtp_remote_key_len) override;
		virtual void onDTLSFailed(DTLSTransport* dtlsTransport) override;
		virtual void onDTLSClosed(DTLSTransport* dtlsTransport) override;
		virtual void onOutgoingDTLSData(RTC::DTLSTransport* dtlsTransport, const uint8_t* data, size_t len) override;
		virtual void onDTLSApplicationData(RTC::DTLSTransport* dtlsTransport, const uint8_t* data, size_t len) override;

	public:
		// Passed by argument.
		uint32_t transportId;

	protected:
		// Others.
		bool hasIPv4udp = false;
		bool hasIPv6udp = false;
		bool hasIPv4tcp = false;
		bool hasIPv6tcp = false;

	private:
		// Passed by argument.
		Listener* listener = nullptr;
		Channel::Notifier* notifier = nullptr;
		// Allocated by this.
		RTC::ICEServer* iceServer = nullptr;
		std::vector<RTC::UDPSocket*> udpSockets;
		std::vector<RTC::TCPServer*> tcpServers;
		RTC::DTLSTransport* dtlsTransport = nullptr;
		// Others.
		bool allocated = false;
		// Others (ICE).
		std::vector<IceCandidate> iceLocalCandidates;
		RTC::TransportTuple* selectedTuple = nullptr;
		// Others (DTLS).
		bool remoteDtlsParametersGiven = false;
		RTC::DTLSTransport::Role dtlsLocalRole = RTC::DTLSTransport::Role::AUTO;
	};

	/* Inline methods. */

	inline
	std::string& Transport::GetIceUsernameFragment()
	{
		return this->iceServer->GetUsernameFragment();
	}

	inline
	std::string& Transport::GetIcePassword()
	{
		return this->iceServer->GetPassword();
	}
}

#endif
