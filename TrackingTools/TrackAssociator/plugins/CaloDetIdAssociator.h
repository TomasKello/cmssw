#ifndef TrackAssociator_CaloDetIdAssociator_h
#define TrackAssociator_CaloDetIdAssociator_h 1
// -*- C++ -*-
//
// Package:    TrackAssociator
// Class:      CaloDetIdAssociator
//
/*

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Dmytro Kovalskyi
//         Created:  Fri Apr 21 10:59:41 PDT 2006
//
//

#include "TrackingTools/TrackAssociator/interface/DetIdAssociator.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"
#include "DataFormats/DetId/interface/DetId.h"

class CaloDetIdAssociator : public DetIdAssociator {
public:
  CaloDetIdAssociator() : DetIdAssociator(72, 70, 0.087), geometry_(nullptr) {}
  CaloDetIdAssociator(const int nPhi, const int nEta, const double etaBinSize, CaloGeometry const* geom)
      : DetIdAssociator(nPhi, nEta, etaBinSize), geometry_(geom) {}

  const GeomDet* getGeomDet(const DetId& id) const override { return nullptr; };

  const char* name() const override { return "CaloTowers"; }

protected:
  void check_setup() const override;

  GlobalPoint getPosition(const DetId& id) const override;

  void getValidDetIds(unsigned int subDetectorIndex, std::vector<DetId>&) const override;

  std::pair<const_iterator, const_iterator> getDetIdPoints(const DetId& id,
                                                           std::vector<GlobalPoint>& points) const override;

  bool insideElement(const GlobalPoint& point, const DetId& id) const override {
    return geometry_->getSubdetectorGeometry(id)->getGeometry(id)->inside(point);
  };

  bool crossedElement(const GlobalPoint&,
                      const GlobalPoint&,
                      const DetId& id,
                      const double tolerance = -1,
                      const SteppingHelixStateInfo* = nullptr) const override;
  const CaloGeometry* geometry_;
  std::vector<GlobalPoint> dummy_;
};
#endif
