set(GravityProvisioningPlugin_SRCS
    ProvisioningPlugin.cpp
    Provisioning.cpp
)


hyperspace_add_qt5_consumer(GravityProvisioningPlugin_SRCS ${CMAKE_SOURCE_DIR}/share/com.ispirata.Hemera.Provisioning.json Provisioning.h Provisioning)

hyperspace_add_qt5_producer(GravityProvisioningPlugin_SRCS ${CMAKE_SOURCE_DIR}/share/com.ispirata.Hemera.Provisioning.Events.json ProvisioningEventsProducer ProvisioningEventsProducer)

# final lib
add_library(gravity-center-plugin-provisioning SHARED ${GravityProvisioningPlugin_SRCS})

target_link_libraries(gravity-center-plugin-provisioning
                      Gravity::Supermassive
                      HemeraQt5SDK::Core
                      HyperspaceQt5::Core HyperspaceQt5::ProducerConsumer
                      Qt5::Core Qt5::Network Qt5::DBus
                      ${ZYPP_LIBRARY})

# Install phase
install(TARGETS gravity-center-plugin-provisioning
        RUNTIME DESTINATION "${GRAVITY_PLUGINS_DIR}" COMPONENT bin
        LIBRARY DESTINATION "${GRAVITY_PLUGINS_DIR}" COMPONENT shlib
        ARCHIVE DESTINATION "${GRAVITY_PLUGINS_DIR}" COMPONENT shlib
        COMPONENT gravity-provisioning)
