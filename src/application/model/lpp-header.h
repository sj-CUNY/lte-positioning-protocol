#ifndef LPP_HEADER_H
#define LPP_HEADER_H

#include <stdint.h>
#include <vector>
#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"


namespace ns3 {

class LppHeader : public Header
{
public:
  LppHeader ();
  virtual ~LppHeader ();
  enum packet_subtype
  {
	CAPABILITIES,
        ASSISTANCE_DATA,
        LOCATION_INFO,
        CERTIFICATE,
        ERROR,
        FATAL
  };

  enum packet_type
  {
	REQUEST,
        PROVIDE,
        ABORT 
  };


  void SendLppPacket(Ptr<Socket> socket, LppHeader::packet_type type, LppHeader::packet_subtype subtype, Address from, uint8_t flag = 0);
  void random_data(uint8_t *s, int size);
  /**
   * Set the packet total length.
   * \param length The packet length.
   */
  void SetPacketLength (uint16_t length)
  {
    m_packetLength = length;
  }

  /**
   * Get the packet total length.
   * \return The packet length.
   */
  uint16_t GetPacketLength () const
  {
    return m_packetLength;
  }

  /**
   * Set the packet type.
   * \param t The packet type.
   */
  void SetPacketType (packet_type t)
  {
    m_packetType = t;
  }

  /**
   * Get the packet sequence number.
   * \returns The packet sequence number.
   */
  packet_type GetPacketType () const
  {
    return m_packetType;
  }

  /**
   * Set the packet type.
   * \param t The packet type.
   */
  void SetPacketSubType (packet_subtype t)
  {
    m_packetSubType = t;
  }
  void SetPacketTime (uint32_t t)
  {
    m_packetTime = t;
  }

  void SetFlag (uint8_t f)
  {
    m_flag = f;
  }
  /**
   * Get the packet sequence number.
   * \returns The packet sequence number.
   */
  packet_subtype GetPacketSubType () const
  {
    return m_packetSubType;
  }

  int64_t GetPacketTime () const
  {
    return m_packetTime;
  }

  uint8_t GetFlag() const
  {
    return m_flag;
  }
private:
  uint8_t *m_data;
  uint8_t m_flag; //more packets or not
  uint32_t m_dataSize;
  uint16_t m_packetLength;          //!< The packet length.
  packet_type m_packetType;  //!< The packet sequence number.
  packet_subtype m_packetSubType;  //!< The packet sequence number.
  int64_t m_packetTime;

public:
  /**
   * \brief Get the type ID.
   * \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
};
}
#endif /* LPP_HEADER_H */

