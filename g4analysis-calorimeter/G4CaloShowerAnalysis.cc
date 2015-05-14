#include "G4CaloShowerAnalysis.h"

#include <boost/foreach.hpp>
#include <map>
#include <sstream>

/* g4main includes */
#include <g4main/PHG4TruthInfoContainer.h>
#include <g4main/PHG4HitContainer.h>
#include <g4main/PHG4Particle.h>
#include <g4main/PHG4Hit.h>

/* Fun4All includes */
#include <fun4all/Fun4AllHistoManager.h>
#include <fun4all/Fun4AllReturnCodes.h>

#include <fun4all/getClass.h>

/* ROOT includes */
#include "TFile.h"
#include "TH1F.h"
#include "TMath.h"

using namespace std;

G4CaloShowerAnalysis::G4CaloShowerAnalysis(const std::string name , const std::string filename ):
  SubsysReco( name ),
  _truth_info_container(NULL),
  _g4hits(NULL),
  _filename(filename),
  _node_name_truth("G4TruthInfo"),
  _nevent(0)
{
  _store_esum = false;
  _store_lprof = false;
  _store_rprof = false;
}

G4CaloShowerAnalysis::~G4CaloShowerAnalysis()
{
}

int G4CaloShowerAnalysis::Init( PHCompositeNode* topNode )
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

  /* Book histogram for longitudinal shower profile */
  if ( _store_lprof )
    {
      _h_lprof = new TH1F( "h_lprof" , "" ,  _h_lprof_bins, _h_lprof_xmin, _h_lprof_xmax );
      _h_lprof->GetXaxis()->SetTitle("z [m]");
      _h_lprof->GetYaxis()->SetTitle("<E> [GeV]");
    }

  /* Book histogram for radial shower profile */
  if ( _store_rprof )
    {
      _h_rprof = new TH1F( "h_rprof" , "" ,  _h_rprof_bins, _h_rprof_xmin, _h_rprof_xmax );
      _h_rprof->GetXaxis()->SetTitle("r [m]");
      _h_rprof->GetYaxis()->SetTitle("<E> [GeV]");
    }

  return 0;
}

int G4CaloShowerAnalysis::process_event( PHCompositeNode* topNode )
{
  /* Increment event counter */
  _nevent++;

  /* List of full event parameters */
  float event_esum = 0;

  //  /* List of single hit parameters */
  //  float event_esum = 0;

  /* Get the Geant4 Truth particle information container */
  _truth_info_container = findNode::getClass<PHG4TruthInfoContainer>(topNode,_node_name_truth.c_str());
  if(!_truth_info_container)
    {
      cout << PHWHERE << " WARNING: Can't find PHG4TruthInfoContainer." << endl;
      return -1;//ABORTEVENT;
    }

  /* Get G4Hits from input nodes */
  unsigned nnodes = _node_names.size();

  if ( !nnodes )
    {
      cout << PHWHERE << " WARNING: No PHG4HitContainer defined." << endl;
      return -1;//ABORTEVENT;
    }

  /* Loop over all input nodes for G4Hits */
  for (unsigned i = 0; i < nnodes; i++)
    {
      PHG4HitContainer *_g4hits = findNode::getClass<PHG4HitContainer>(topNode, _node_names.at(i).c_str());

      if (_g4hits)
	{
	  PHG4HitContainer::ConstRange hit_range = _g4hits->getHits();

	  for (PHG4HitContainer::ConstIterator hit_iter = hit_range.first ; hit_iter !=  hit_range.second; hit_iter++ )
	    {
	      float edep = hit_iter->second->get_edep();
	      float x0   = hit_iter->second->get_x(0);
	      float y0   = hit_iter->second->get_y(0);
	      float z0 = hit_iter->second->get_z(0);
	      float x1   = hit_iter->second->get_x(1);
	      float y1   = hit_iter->second->get_y(1);
	      float z1 = hit_iter->second->get_z(1);
	      float x = 0.5*(x0+x1);
	      float y = 0.5*(y0+y1);
	      float z = 0.5*(z0+z1);

	      event_esum += edep;

	      /* Store single-hit values */
	      if ( _store_lprof )
		{
		  _h_lprof->Fill( z , edep );
		}

	      if ( _store_rprof )
		{
		  _h_rprof->Fill( TMath::Sqrt( x*x + y*y ) , edep );
		}
	    }
	}
    }

  /* Store full-event values */
  if ( _store_esum )
    {
      _h_esum->Fill( event_esum );
    }

  return 0;
}

int G4CaloShowerAnalysis::End(PHCompositeNode * topNode)
{
  /* Select output file */
  _outfile->cd();

  /* Write histograms to output file */
  if ( _h_esum )
    _h_esum->Write();

  if ( _h_lprof )
    _h_lprof->Write();

  if ( _h_rprof )
    _h_rprof->Write();

  /* Write & Close output file */
  _outfile->Write();
  _outfile->Close();

  return 0;
}