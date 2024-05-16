include $(TOPDIR)/rules.mk

PKG_NAME:=mqtt-subscriber
PKG_RELEASE:=1
PKG_VERSION:=1.0.0

PKG_BUILD_DEPENDS:=!USE_GLIBC:argp-standalone

TARGET_CFLAGS += -Og -ggdb3

include $(INCLUDE_DIR)/package.mk

define Package/mqtt-subscriber
	CATEGORY:=Examples
	TITLE:=mqtt-subscriber
	DEPENDS:= \
		+libcurl \
		+libmosquitto \
		+libuci
endef

define Package/mqtt-subscriber/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mqtt_sub $(1)/usr/bin
	$(INSTALL_CONF) ./files/mqtt_sub.config $(1)/etc/config/mqtt_sub
endef

$(eval $(call BuildPackage,mqtt-subscriber))
