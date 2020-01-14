//---------------------------------------------------------------------------
// Created 24-Nov-2007: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
// Copyright (C) Thomas Mulgrew
//
// Abstract interface to application settings
#ifndef AppSettingsH
#define AppSettingsH

////////////////////////////////////////////////////////////////////////////////
//  IAppSettings
//
/// Interface to application settings used by routines and function libraries
class IAppSettings {
public:

    /// Returns true if application is in sandbox mode.
    /// This mode limits the amount of damage that a running program can do, and
    /// is intended for running code that may not come from a trusted source.
    /// In sandbox mode, programs may only read and write from the same
    /// directory in which they are running or a subdirectory thereof), and
    /// cannot delete files.
    virtual bool IsSandboxMode() = 0;

    /// Returns the selected language dialect.
    virtual int Syntax() = 0;
};

/// Return the AppSettings object
extern IAppSettings* GetAppSettings();

#endif
