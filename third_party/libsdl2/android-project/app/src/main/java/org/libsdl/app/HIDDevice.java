package org.libsdl.app;

import android.hardware.usb.UsbDevice;

interface HIDDevice
{
    public int getId();
    public int getVendorId();
    public int getProductId();
    public String getSerialNumber();
    public int getVersion();
    public String getManufacturerName();
    public String getProductName();
    public UsbDevice getDevice();
    public boolean open();
    public int sendFeatureReport(byte[] report);
    public int sendOutputReport(byte[] report);
    public boolean getFeatureReport(byte[] report);
    public void setFrozen(boolean frozen);
    public void close();
    public void shutdown();
}
