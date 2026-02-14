#ifndef COM_HPP
#define COM_HPP

#include <string>
#include <vector>
#include <memory>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

using boost::asio::ip::tcp;

class Com : public boost::enable_shared_from_this<Com> {
public:
    typedef boost::shared_ptr<Com> pointer;

    static pointer create(boost::asio::io_context& io_context);

    boost::asio::serial_port::native_handle_type native_handle();

    /// Initialize serial communication
    /// @param port Serial port name (e.g., "COM1", "/dev/ttyUSB0")
    /// @param baudrate Baud rate (e.g., 9600, 115200)
    /// @return true if successful, false otherwise
    bool init(const std::string& port, unsigned int baudrate);

    /// Read data from serial port
    /// @param buffer Buffer to store read data
    /// @param size Maximum number of bytes to read
    /// @return Number of bytes actually read, -1 on error
    int serialRead(uint8_t* buffer, size_t size);

    /// Write data to serial port
    /// @param buffer Data to write
    /// @param size Number of bytes to write
    /// @return Number of bytes actually written, -1 on error
    int serialWrite(const uint8_t* buffer, size_t size);

    /// Close serial connection
    void close();

    /// Check if serial port is open
    /// @return true if open, false otherwise
    bool isOpen() const;

private:
    Com(boost::asio::io_context& io_context);

    boost::asio::io_context& io_context_;
    std::unique_ptr<boost::asio::serial_port> serial_port_;
    bool is_open_;
};

#endif // COM_HPP
