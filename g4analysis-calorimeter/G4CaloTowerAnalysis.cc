#include "G4CaloTowerAnalysis.h"

#include <boost/foreach.hpp>
#include <map>
#include <sstream>

/* PHG4 includes */
#include <g4main/PHG4TruthInfoContainer.h>

#include <g4main/PHG4Particle.h>
//#include <g4main/PHMCTowerContainerV1.h>
//#include <g4main/PHMCTower.h>

/* Fun4All includes */
#include <fun4all/Fun4AllHistoManager.h>
#include <fun4all/Fun4AllReturnCodes.h>

#include <fun4all/getClass.h>

/* ROOT includes */
#include "TFile.h"
#include "TH1F.h"
#include "TMath.h"

using namespace std;

G4CaloTowerAnalysis::G4CaloTowerAnalysis(const std::string name , const std::string filename ):
  SubsysReco( name ),
  _truth_info_container(NULL),
  _tower(NULL),
  _filename(filename),
  _node_name_truth("G4TruthInfo"),
  _nevent(0)
{
  _store_esum = false;
}

G4CaloTowerAnalysis::~G4CaloTowerAnalysis()
{
}

int G4CaloTowerAnalysis::Init( PHCompositeNode* topNode )
{

  /* Create new output file */
  _outfile = new TFile(_filename.c_str(), "RECREATE");

  /* Book histogram for total energy sum */
  if ( _store_esum )
    {
      _h_esum = new TH1F( "h_esum" , "" ,  _h_esum_bins, _h_esum_xmin, _h_esum_xmax );
      _h_esum->GetXaxis()->SetTitle("E [GeV]");
      _h_esum->GetYaxis()->SetTitle("# Entries / #Sigma Entries");
    }

  return 0;
}

int G4CaloTowerAnalysis::process_event( PHCompositeNode* topNode )
{
  /* Increment event counter */
  _nevent++;

  /* List of full event parameters */
  float event_esum = 0;

  /* Get the Geant4 Truth particle information container */
  _truth_info_container = findNode::getClass<PHG4TruthInfoContainer>(topNode,_node_name_truth.c_str());
  if(!_truth_info_container)
    {
      cout << PHWHERE << " WARNING: Can't find PHG4TruthInfoContainer." << endl;
      return -1;//ABORTEVENT;
    }

  /* Get Tower from input nodes */
  unsigned nnodes = _node_tower_names.size();

  if ( !nnodes )
    {
      cout << PHWHERE << " WARNING: No tower input node defined." << endl;
      return -1;//ABORTEVENT;
    }

  /* Loop over all input nodes for tower */
  for (unsigned i = 0; i < nnodes; i++)
    {
//      PHMCTowerContainer *_tower = findNode::getClass<PHMCTowerContainerV1>(topNode, _node_tower_names.at(i).c_str());
//
//      if (_tower)
//	{
//	  unsigned ntower = _tower->size();
//
//	  for ( unsigned itower = 0; itower < ntower; itower++ )
//	    {
//	      PHMCTower *tower = _tower->getTower(itower);
//
//	      double energyT = tower->GetEt();
//	      double eta = tower->GetEta();
//	      //double phi = tower->GetPhi();
//
//	      double pz = energyT * sinh( eta );
//	      double energy = sqrt( energyT * energyT + pz * pz );
//
//	      event_esum += energy;
//
//	      /* Store single-tower values */
//	      // ...
//	    }
//	}
    }

  /* Store full-event values */
  if ( _store_esum )
    {
      _h_esum->Fill( event_esum );
    }

  return 0;
}

int G4CaloTowerAnalysis::End(PHCompositeNode * topNode)
{
  /* Select output file */
  _outfile->cd();

  /* Write histograms to output file */
  if ( _h_esum )
    _h_esum->Write();

  /* Write & Close output file */
  _outfile->Write();
  _outfile->Close();

  return 0;
}