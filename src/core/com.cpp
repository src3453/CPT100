// Serial communication implementation

#include "header/com.hpp"
#include <iostream>

Com::Com(boost::asio::io_context& io_context)
    : io_context_(io_context), is_open_(false) {
}

Com::pointer Com::create(boost::asio::io_context& io_context) {
    return pointer(new Com(io_context));
}

boost::asio::serial_port::native_handle_type Com::native_handle() {
    if (serial_port_) {
        return serial_port_->native_handle();
    }
    // Return invalid handle based on platform
    #ifdef _WIN32
        return INVALID_HANDLE_VALUE;
    #else
        return -1;
    #endif
}

bool Com::init(const std::string& port, unsigned int baudrate) {
    try {
        serial_port_ = std::make_unique<boost::asio::serial_port>(io_context_);

        // Open the serial port
        serial_port_->open(port);

        // Set serial port options
        serial_port_->set_option(boost::asio::serial_port_base::baud_rate(baudrate));
        serial_port_->set_option(boost::asio::serial_port_base::character_size(8));
        serial_port_->set_option(boost::asio::serial_port_base::stop_bits(
            boost::asio::serial_port_base::stop_bits::one));
        serial_port_->set_option(boost::asio::serial_port_base::parity(
            boost::asio::serial_port_base::parity::none));
        serial_port_->set_option(boost::asio::serial_port_base::flow_control(
            boost::asio::serial_port_base::flow_control::none));

        is_open_ = true;
        std::cout << "Serial port " << port << " opened successfully at " << baudrate << " baud" << std::endl;
        return true;
    }
    catch (std::exception& e) {
        std::cerr << "Error opening serial port: " << e.what() << std::endl;
        is_open_ = false;
        return false;
    }
}

int Com::serialRead(uint8_t* buffer, size_t size) {
    if (!is_open_ || !serial_port_) {
        std::cerr << "Serial port is not open" << std::endl;
        return -1;
    }

    try {
        size_t bytes_read = serial_port_->read_some(boost::asio::buffer(buffer, size));
        return static_cast<int>(bytes_read);
    }
    catch (std::exception& e) {
        std::cerr << "Error reading from serial port: " << e.what() << std::endl;
        return -1;
    }
}

int Com::serialWrite(const uint8_t* buffer, size_t size) {
    if (!is_open_ || !serial_port_) {
        std::cerr << "Serial port is not open" << std::endl;
        return -1;
    }

    try {
        size_t bytes_written = boost::asio::write(*serial_port_, boost::asio::buffer(buffer, size));
        return static_cast<int>(bytes_written);
    }
    catch (std::exception& e) {
        std::cerr << "Error writing to serial port: " << e.what() << std::endl;
        return -1;
    }
}

void Com::close() {
    if (serial_port_ && is_open_) {
        try {
            serial_port_->close();
            is_open_ = false;
            std::cout << "Serial port closed" << std::endl;
        }
        catch (std::exception& e) {
            std::cerr << "Error closing serial port: " << e.what() << std::endl;
        }
    }
}

bool Com::isOpen() const {
    return is_open_;
}
