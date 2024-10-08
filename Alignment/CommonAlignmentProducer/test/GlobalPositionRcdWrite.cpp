
#include <string>
#include <map>
#include <vector>

// Framework
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "DataFormats/DetId/interface/DetId.h"

// Database
#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"

// Alignment
#include "Alignment/CommonAlignment/interface/Utilities.h"
#include "CondFormats/Alignment/interface/Alignments.h"
#include "CondFormats/Alignment/interface/AlignTransform.h"
#include "CondFormats/AlignmentRecord/interface/GlobalPositionRcd.h"

class GlobalPositionRcdWrite : public edm::one::EDAnalyzer<> {
public:
  explicit GlobalPositionRcdWrite(const edm::ParameterSet& iConfig)
      : m_useEulerAngles(iConfig.getParameter<bool>("useEulerAngles")),
        m_tracker(iConfig.getParameter<edm::ParameterSet>("tracker")),
        m_muon(iConfig.getParameter<edm::ParameterSet>("muon")),
        m_ecal(iConfig.getParameter<edm::ParameterSet>("ecal")),
        m_hcal(iConfig.getParameter<edm::ParameterSet>("hcal")),
        m_calo(iConfig.getParameter<edm::ParameterSet>("calo")),
        nEventCalls_(0) {}
  ~GlobalPositionRcdWrite() {}
  virtual void analyze(const edm::Event& evt, const edm::EventSetup& evtSetup);

private:
  AlignTransform::Rotation toMatrix(double alpha, double beta, double gamma);

  bool m_useEulerAngles;
  edm::ParameterSet m_tracker, m_muon, m_ecal, m_hcal, m_calo;
  unsigned int nEventCalls_;
};

AlignTransform::Rotation GlobalPositionRcdWrite::toMatrix(double alpha, double beta, double gamma) {
  align::EulerAngles angles(3);
  angles(1) = alpha;
  angles(2) = beta;
  angles(3) = gamma;

  align::RotationType alignRotation = align::toMatrix(angles);

  return AlignTransform::Rotation(CLHEP::HepRep3x3(alignRotation.xx(),
                                                   alignRotation.xy(),
                                                   alignRotation.xz(),
                                                   alignRotation.yx(),
                                                   alignRotation.yy(),
                                                   alignRotation.yz(),
                                                   alignRotation.zx(),
                                                   alignRotation.zy(),
                                                   alignRotation.zz()));
}

void GlobalPositionRcdWrite::analyze(const edm::Event& evt, const edm::EventSetup& iSetup) {
  if (nEventCalls_ > 0) {
    edm::LogPrint("GlobalPositionRcdWrite") << "Writing to DB to be done only once, "
                                            << "set 'untracked PSet maxEvents = {untracked int32 input = 1}'."
                                            << "(Your writing should be fine.)" << std::endl;
    return;
  }

  Alignments globalPositions{};

  AlignTransform tracker(AlignTransform::Translation(m_tracker.getParameter<double>("x"),
                                                     m_tracker.getParameter<double>("y"),
                                                     m_tracker.getParameter<double>("z")),
                         (m_useEulerAngles != true)
                             ? this->toMatrix(m_tracker.getParameter<double>("alpha"),
                                              m_tracker.getParameter<double>("beta"),
                                              m_tracker.getParameter<double>("gamma"))
                             : AlignTransform::EulerAngles(m_tracker.getParameter<double>("alpha"),
                                                           m_tracker.getParameter<double>("beta"),
                                                           m_tracker.getParameter<double>("gamma")),
                         DetId(DetId::Tracker).rawId());

  AlignTransform muon(
      AlignTransform::Translation(
          m_muon.getParameter<double>("x"), m_muon.getParameter<double>("y"), m_muon.getParameter<double>("z")),
      (m_useEulerAngles != true) ? this->toMatrix(m_muon.getParameter<double>("alpha"),
                                                  m_muon.getParameter<double>("beta"),
                                                  m_muon.getParameter<double>("gamma"))
                                 : AlignTransform::EulerAngles(m_muon.getParameter<double>("alpha"),
                                                               m_muon.getParameter<double>("beta"),
                                                               m_muon.getParameter<double>("gamma")),
      DetId(DetId::Muon).rawId());

  AlignTransform ecal(
      AlignTransform::Translation(
          m_ecal.getParameter<double>("x"), m_ecal.getParameter<double>("y"), m_ecal.getParameter<double>("z")),
      (m_useEulerAngles != true) ? this->toMatrix(m_ecal.getParameter<double>("alpha"),
                                                  m_ecal.getParameter<double>("beta"),
                                                  m_ecal.getParameter<double>("gamma"))
                                 : AlignTransform::EulerAngles(m_ecal.getParameter<double>("alpha"),
                                                               m_ecal.getParameter<double>("beta"),
                                                               m_ecal.getParameter<double>("gamma")),
      DetId(DetId::Ecal).rawId());

  AlignTransform hcal(
      AlignTransform::Translation(
          m_hcal.getParameter<double>("x"), m_hcal.getParameter<double>("y"), m_hcal.getParameter<double>("z")),
      (m_useEulerAngles != true) ? this->toMatrix(m_hcal.getParameter<double>("alpha"),
                                                  m_hcal.getParameter<double>("beta"),
                                                  m_hcal.getParameter<double>("gamma"))
                                 : AlignTransform::EulerAngles(m_hcal.getParameter<double>("alpha"),
                                                               m_hcal.getParameter<double>("beta"),
                                                               m_hcal.getParameter<double>("gamma")),
      DetId(DetId::Hcal).rawId());

  AlignTransform calo(
      AlignTransform::Translation(
          m_calo.getParameter<double>("x"), m_calo.getParameter<double>("y"), m_calo.getParameter<double>("z")),
      (m_useEulerAngles != true) ? this->toMatrix(m_calo.getParameter<double>("alpha"),
                                                  m_calo.getParameter<double>("beta"),
                                                  m_calo.getParameter<double>("gamma"))
                                 : AlignTransform::EulerAngles(m_calo.getParameter<double>("alpha"),
                                                               m_calo.getParameter<double>("beta"),
                                                               m_calo.getParameter<double>("gamma")),
      DetId(DetId::Calo).rawId());

  edm::LogPrint("GlobalPositionRcdWrite")
      << "\nProvided rotation angles are interpreted as "
      << ((m_useEulerAngles != true) ? "rotations around X, Y and Z" : "Euler angles") << ".\n"
      << std::endl;

  edm::LogPrint("GlobalPositionRcdWrite") << "Tracker (" << tracker.rawId() << ") at " << tracker.translation() << " "
                                          << tracker.rotation().eulerAngles() << std::endl;
  edm::LogPrint("GlobalPositionRcdWrite") << tracker.rotation() << std::endl;

  edm::LogPrint("GlobalPositionRcdWrite")
      << "Muon (" << muon.rawId() << ") at " << muon.translation() << " " << muon.rotation().eulerAngles() << std::endl;
  edm::LogPrint("GlobalPositionRcdWrite") << muon.rotation() << std::endl;

  edm::LogPrint("GlobalPositionRcdWrite")
      << "Ecal (" << ecal.rawId() << ") at " << ecal.translation() << " " << ecal.rotation().eulerAngles() << std::endl;
  edm::LogPrint("GlobalPositionRcdWrite") << ecal.rotation() << std::endl;

  edm::LogPrint("GlobalPositionRcdWrite")
      << "Hcal (" << hcal.rawId() << ") at " << hcal.translation() << " " << hcal.rotation().eulerAngles() << std::endl;
  edm::LogPrint("GlobalPositionRcdWrite") << hcal.rotation() << std::endl;

  edm::LogPrint("GlobalPositionRcdWrite")
      << "Calo (" << calo.rawId() << ") at " << calo.translation() << " " << calo.rotation().eulerAngles() << std::endl;
  edm::LogPrint("GlobalPositionRcdWrite") << calo.rotation() << std::endl;

  globalPositions.m_align.push_back(tracker);
  globalPositions.m_align.push_back(muon);
  globalPositions.m_align.push_back(ecal);
  globalPositions.m_align.push_back(hcal);
  globalPositions.m_align.push_back(calo);

  edm::LogPrint("GlobalPositionRcdWrite") << "Uploading to the database..." << std::endl;

  edm::Service<cond::service::PoolDBOutputService> poolDbService;

  if (!poolDbService.isAvailable())
    throw cms::Exception("NotAvailable") << "PoolDBOutputService not available";

  //    if (poolDbService->isNewTagRequest("GlobalPositionRcd")) {
  //       poolDbService->createOneIOV<Alignments>(globalPositions, poolDbService->endOfTime(), "GlobalPositionRcd");
  //    } else {
  //       poolDbService->appendOneIOV<Alignments>(globalPositions, poolDbService->currentTime(), "GlobalPositionRcd");
  //    }
  poolDbService->writeOneIOV<Alignments>(globalPositions, poolDbService->currentTime(), "GlobalPositionRcd");
  edm::LogPrint("GlobalPositionRcdWrite") << "done!" << std::endl;
  nEventCalls_++;
}

//define this as a plug-in
DEFINE_FWK_MODULE(GlobalPositionRcdWrite);
