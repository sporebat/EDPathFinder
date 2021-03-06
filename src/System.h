//
//  Copyright (C) 2016-2017  David Hedbor <neotron@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#pragma once

#include <cmath>
#include <QString>
#include <AStar.h>
#include <QVector3D>
#include <QJsonObject>
#include <QThread>
#include <QDebug>
#include <QUrl>
#include <base/integral_types.h>
#include <QJsonDocument>

class AStarSystemNode;

class AStarRouter;

class Settlement;

class Planet;

class System;

typedef QList<Settlement> SettlementList;
typedef QList<Planet>     PlanetList;
typedef QList<System>     SystemList;

enum ThreatLevel {
    ThreatLevelUnknown                = 0,
    ThreatLevelLow                    = 1 << 0,
    ThreatLevelRestrictedLongDistance = 1 << 1,
    ThreatLevelMedium                 = 1 << 2,
    ThreatLeveLHigh                   = 1 << 3,
};

enum SettlementSize {
    SettlementSizeSmall = 1 << 0, SettlementSizeMedium = 1 << 1, SettlementSizeLarge = 1 << 2
};

enum SettlementFlags {
    SettlementFlagsCoreDataTerminal          = 1 << 0,
    SettlementFlagsJumpClimbRequired         = 1 << 1,
    SettlementFlagsClassifiedScanDatabanks   = 1 << 2,
    SettlementFlagsClassifiedScanFragment    = 1 << 3,
    SettlementFlagsCrackedIndustrialFirmware = 1 << 4,
    SettlementFlagsDivergentScanData         = 1 << 5,
    SettlementFlagsModifiedConsumerFirmware  = 1 << 6,
    SettlementFlagsModifiedEmbeddedFirmware  = 1 << 7,
    SettlementFlagsOpenSymmetricKeys         = 1 << 8,
    SettlementFlagsSecurityFirmwarePatch     = 1 << 9,
    SettlementFlagsSpecializedLegacyFirmware = 1 << 10,
    SettlementFlagsTaggedEncryptionCodes     = 1 << 11,
    SettlementFlagsUnusualEncryptedFiles     = 1 << 12,
    SettlementFlagsAnarchy                   = 1 << 13
};

class SettlementType {
public:
    static const QString IMAGE_BASE_ICON;
    static const QString IMAGE_SATELLITE;
    static const QString IMAGE_COREFULLMAP;
    static const QString IMAGE_OVERVIEW;
    static const QString IMAGE_PATHMAP;
    static const QString IMAGE_OVERVIEW3D;
    static const QString IMAGE_CORE;


    SettlementType(SettlementSize size, ThreatLevel securityLevel, const QString &economy) : _size(size),
                                                                                             _securityLevel(
                                                                                                     securityLevel),
                                                                                             _economy(economy),
                                                                                             _images() { }

    SettlementType(const SettlementType &other) : _size(other._size), _securityLevel(other._securityLevel),
                                                  _economy(other._economy), _images(other._images) { }

    SettlementType(const SettlementType &&other) : _size(other._size), _securityLevel(other._securityLevel),
                                                   _economy(std::move(other._economy)),
                                                   _images(std::move(other._images)) { }


    SettlementType() { }

    SettlementSize size() const {
        return _size;
    }

    ThreatLevel securityLevel() const {
        return _securityLevel;
    }

    const QString &economy() const {
        return _economy;
    }


    const QUrl imageNamed(const QString &name) const {
        return _images.contains(name) ? _images[name] : QUrl();
    }

    SettlementType &operator=(const SettlementType &&other) {
        _size          = other._size;
        _securityLevel = other._securityLevel;
        _economy       = std::move(other._economy);
        _images        = std::move(other._images);
        return *this;
    }

    SettlementType &operator=(const SettlementType &other) {
        _size          = other._size;
        _securityLevel = other._securityLevel;
        _economy       = other._economy;
        _images        = other._images;
        return *this;
    }

    void addImage(const QString &name, const QUrl &url) {
        if(url.isValid()) {
            _images[name] = url;
        }
    }

    const QStringList imageTitles() const {
        auto keys = _images.keys();
        keys.removeAll(SettlementType::IMAGE_BASE_ICON);
        keys.sort();
        return keys;
    }

private:
    SettlementSize _size;
    ThreatLevel    _securityLevel;
    QString        _economy;

    QMap<QString, QUrl> _images;
};

class Settlement {
public:


    Settlement(const QString &name, int32 flags = 0, ThreatLevel threatLevel = ThreatLevelLow,
               const SettlementType *type = nullptr) : _name(name), _flags(flags), _threatLevel(threatLevel),
                                                       _type(type) { }

    Settlement(const Settlement &&other) : _name(std::move(other._name)), _flags(other._flags),
                                           _threatLevel(other._threatLevel), _type(other._type) {
    }

    Settlement(const Settlement &other) : _name(other._name), _flags(other._flags), _threatLevel(other._threatLevel),
                                          _type(other._type) {
    }

    Settlement &operator=(const Settlement &&other) {
        _name        = std::move(other._name);
        _flags       = other._flags;
        _threatLevel = other._threatLevel;
        _type        = other._type;
        return *this;
    }

    Settlement &operator=(const Settlement &other) {
        _name        = other._name;
        _flags       = other._flags;
        _threatLevel = other._threatLevel;
        _type        = other._type;
        return *this;
    }

    const QString &name() const {
        return _name;
    }

    SettlementSize size() const {
        return _type->size();
    }

    ThreatLevel threatLevel() const {
        return _threatLevel;
    }

    int32 flags() const {
        return _flags;
    }


    const SettlementType *type() const {
        return _type;
    }

private:
    QString              _name;
    int32                _flags;
    ThreatLevel          _threatLevel;
    const SettlementType *_type;
};

class Planet {
public:
    Planet() : _name(), _distance(0), _settlements() { }

    Planet(const QString &name, int distance, const Settlement &settlement) : _name(name), _distance(distance),
                                                                              _settlements() {
        _settlements.push_back(settlement);
    }

    Planet(const QString &name, int distance, const SettlementList &settlements) : _name(name), _distance(distance),
                                                                                   _settlements(settlements) { }

    Planet(const Planet &&other) : _name(std::move(other._name)), _distance(other._distance),
                                   _settlements(std::move(other._settlements)) { }

    Planet(const Planet &other) : _name(other._name), _distance(other._distance), _settlements(other._settlements) { }

    Planet &operator=(const Planet &other) {
        _name        = other._name;
        _settlements = other._settlements;
        _distance    = other._distance;
        return *this;
    }

    Planet &operator=(const Planet &&other) {
        _name        = std::move(other._name);
        _settlements = std::move(other._settlements);
        _distance    = other._distance;
        return *this;
    }

    const SettlementList &settlements() const {
        return _settlements;
    }

    void addSettlement(const Settlement &settlement) {
        _settlements.push_back(settlement);
    }

    const QString &name() const {
        return _name;
    }

    int distance() const {
        return _distance;
    }

private:
    QString        _name;
    int            _distance;
    SettlementList _settlements;
};

class System {

public:

    System() { }

    System(const QString &name, float x, float y, float z) : _name(name), _position(x, y, z) { }

    System(const QString &name, const Planet &planet, float x, float y, float z) : _name(name), _planets(),
                                                                                   _position(x, y, z) {
        _planets.push_back(planet);
    }

    System(const QString &system, const PlanetList &planets, const QVector3D &position) : _name(system),
                                                                                          _planets(planets),
                                                                                          _position(position) { }

    System(const QString &name, const QVector3D &position) : _name(name), _position(position) { }

    System(const System &other) : _name(other._name), _planets(other._planets), _position(other._position) { }

    System(const System &&other) : _name(std::move(other._name)), _planets(std::move(other._planets)),
                                   _position(std::move(other._position)) { }

    System(const QJsonObject &jsonObject);

    System &operator=(const System &other) {
        _name     = other._name;
        _planets  = other._planets;
        _position = other._position;
        return *this;
    }

    System &operator=(const System &&other) {
        _name     = std::move(other._name);
        _planets  = std::move(other._planets);
        _position = std::move(other._position);
        return *this;
    }

    System(AStarSystemNode *system);

    virtual ~System();

// Return distance as a fixed point value with two decimals. Used by TSP
    int64 distance(const System &other) const {
        float distance = _position.distanceToPoint(other._position) * 10;
        return (int64) distance;
    }

    static QString formatDistance(int64 dist);

    float z() const {
        return _position.z();
    }

    float y() const {
        return _position.y();
    }

    float x() const {
        return _position.x();
    }

    const QVector3D &position() const {
        return _position;
    }

    const PlanetList &planets() const { return _planets; }

    void addSettlement(const QString &planetName, const Settlement &settlement, int distance);

    const QString &name() const {
        return _name;
    }

protected:

    System(float x, float y, float z) : _position(x, y, z) { }

    float sqr(float val) const { return val * val; }

    QString    _name;
    PlanetList _planets;
    QVector3D  _position;
};

class SystemLoader : public QThread {
Q_OBJECT

public:
    SystemLoader(AStarRouter *router) : QThread(), _router(router) { }

    void run() override;

    virtual ~SystemLoader();

    void loadSettlements();

    void loadSettlementTypes();

    const SystemList &systems() const {
        return _systems;
    }

    const QMap<QString, SettlementType *> &settlementTypes() const {
        return _settlementTypes;
    }

signals:

    void systemsLoaded(const SystemList &systems);

    void progress(int progress);

    void sortingSystems();

public slots:

    void dataDecompressed(const QByteArray &bytes);

private:

    QMap<QString, SettlementType *> _settlementTypes;
    SystemList                      _systems;
    AStarRouter                     *_router;
    QByteArray                      _bytes;
    QJsonObject                     _bodyDistances;

    void loadSystemFromTextFile();

    int getDistance(const QString &system, const QString &planet);
};
