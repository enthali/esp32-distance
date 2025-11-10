# Certificate Generation Component
#
# This component automatically generates self-signed SSL certificates for HTTPS
# during the build process and embeds them into the firmware.

# Certificate files to be generated and embedded
# Generate certificates in build directory to avoid git commits
set(CERT_DIR "${CMAKE_BINARY_DIR}/certs")
set(SERVER_CERT "${CERT_DIR}/server.crt")
set(SERVER_KEY "${CERT_DIR}/server.key")
set(CA_CERT "${CERT_DIR}/ca.crt")

# Create certificate directory if it doesn't exist
file(MAKE_DIRECTORY ${CERT_DIR})

# Function to check if OpenSSL is available
function(check_openssl_available RESULT_VAR)
    find_program(OPENSSL_EXECUTABLE openssl)
    if(OPENSSL_EXECUTABLE)
        set(${RESULT_VAR} TRUE PARENT_SCOPE)
        message(STATUS "OpenSSL found: ${OPENSSL_EXECUTABLE}")
    else()
        set(${RESULT_VAR} FALSE PARENT_SCOPE)
        message(STATUS "OpenSSL not found, will use Python fallback")
    endif()
endfunction()

# Function to generate certificates using OpenSSL
function(generate_certs_openssl)
    message(STATUS "Generating certificates using OpenSSL...")
    
    # Generate CA private key and certificate (combined file)
    execute_process(
        COMMAND ${OPENSSL_EXECUTABLE} req -x509 -newkey rsa:4096 -keyout ${CA_CERT} 
                -out ${CA_CERT} -days 9125 -nodes 
                -subj "/C=US/ST=CA/L=Local/O=ESP32Device/CN=ESP32-Distance-Sensor-CA"
        RESULT_VARIABLE CA_RESULT
        OUTPUT_QUIET ERROR_QUIET
    )
    
    # Generate server private key
    execute_process(
        COMMAND ${OPENSSL_EXECUTABLE} genrsa -out ${SERVER_KEY} 4096
        RESULT_VARIABLE SERVER_KEY_RESULT
        OUTPUT_QUIET ERROR_QUIET
    )
    
    # Generate server certificate signing request
    execute_process(
        COMMAND ${OPENSSL_EXECUTABLE} req -new -key ${SERVER_KEY} 
                -out ${CERT_DIR}/server.csr 
                -subj "/C=US/ST=CA/L=Local/O=ESP32Device/CN=192.168.4.1"
        RESULT_VARIABLE SERVER_CSR_RESULT
        OUTPUT_QUIET ERROR_QUIET
    )
    
    # Generate server certificate (25 years validity)
    execute_process(
        COMMAND ${OPENSSL_EXECUTABLE} x509 -req -in ${CERT_DIR}/server.csr 
                -CA ${CA_CERT} -CAkey ${CA_CERT} -CAcreateserial 
                -out ${SERVER_CERT} -days 9125
        RESULT_VARIABLE SERVER_CERT_RESULT
        OUTPUT_QUIET ERROR_QUIET
    )
    
    # Clean up CSR file
    file(REMOVE ${CERT_DIR}/server.csr ${CERT_DIR}/ca.srl)
    
    if(CA_RESULT EQUAL 0 AND SERVER_KEY_RESULT EQUAL 0 AND 
       SERVER_CSR_RESULT EQUAL 0 AND SERVER_CERT_RESULT EQUAL 0)
        message(STATUS "Certificates generated successfully using OpenSSL")
    else()
        message(FATAL_ERROR "Failed to generate certificates using OpenSSL")
    endif()
endfunction()

# Function to generate certificates using Python
function(generate_certs_python)
    message(STATUS "Generating certificates using Python...")
    
    # Use the Python certificate generation script with proper environment
    set(ENV{CERT_OUTPUT_DIR} ${CERT_DIR})
    
    # Use ESP-IDF project directory (reliable source directory)
    # CMAKE_BINARY_DIR points to build, so go up one level to get project source
    get_filename_component(PROJECT_SOURCE_DIR "${CMAKE_BINARY_DIR}/.." ABSOLUTE)
    set(CERT_SCRIPT "${PROJECT_SOURCE_DIR}/tools/certificates/generate_cert.py")
    
    message(STATUS "Project source: ${PROJECT_SOURCE_DIR}")
    message(STATUS "Certificate script: ${CERT_SCRIPT}")
    message(STATUS "Output directory: ${CERT_DIR}")
    
    execute_process(
        COMMAND python ${CERT_SCRIPT}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        RESULT_VARIABLE PYTHON_RESULT
        OUTPUT_VARIABLE PYTHON_OUTPUT
        ERROR_VARIABLE PYTHON_ERROR
    )
    
    if(PYTHON_RESULT EQUAL 0)
        message(STATUS "Certificates generated successfully using Python")
        if(PYTHON_OUTPUT)
            message(STATUS "Python output: ${PYTHON_OUTPUT}")
        endif()
    else()
        message(FATAL_ERROR "Failed to generate certificates using Python: ${PYTHON_ERROR}")
    endif()
endfunction()

# Check if certificates need to be generated
set(NEED_CERTS FALSE)
if(NOT EXISTS ${SERVER_CERT} OR NOT EXISTS ${SERVER_KEY} OR NOT EXISTS ${CA_CERT})
    set(NEED_CERTS TRUE)
    message(STATUS "Certificate files missing, will generate new ones")
else()
    message(STATUS "Certificate files found")
endif()

# Generate certificates if needed
if(NEED_CERTS)
    check_openssl_available(HAS_OPENSSL)
    
    if(HAS_OPENSSL)
        generate_certs_openssl()
    else()
        generate_certs_python()
    endif()
    
    message(STATUS "Certificate generation completed")
endif()

# Verify certificate files exist before embedding
if(NOT EXISTS ${SERVER_CERT})
    message(FATAL_ERROR "Server certificate not found: ${SERVER_CERT}")
endif()
if(NOT EXISTS ${SERVER_KEY})
    message(FATAL_ERROR "Server private key not found: ${SERVER_KEY}")
endif()
if(NOT EXISTS ${CA_CERT})
    message(FATAL_ERROR "CA certificate not found: ${CA_CERT}")
endif()

# Register component with embedded certificate files (match existing pattern)
idf_component_register(
    SRCS "cert_handler.c"
    INCLUDE_DIRS "."
    EMBED_FILES 
        ${SERVER_CERT}
        ${SERVER_KEY}
        ${CA_CERT}
    REQUIRES esp_http_server mbedtls
)

# Add build information
message(STATUS "Certificate component configured:")
message(STATUS "  Server Certificate: ${SERVER_CERT}")
message(STATUS "  Server Private Key: ${SERVER_KEY}")
message(STATUS "  CA Certificate: ${CA_CERT}")
message(STATUS "  Files will be embedded in firmware")