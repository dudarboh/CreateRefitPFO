#include "CreateRefitPFO.hpp"
#include "IMPL/LCCollectionVec.h"
#include "IMPL/ReconstructedParticleImpl.h"
#include "IMPL/ParticleIDImpl.h"
#include "EVENT/MCParticle.h"
#include "GeometryUtil.h"
#include "marlin/VerbosityLevels.h"
#include "TMatrixD.h"
using namespace lcio;

CreateRefitPFO aCreateRefitPFO;

CreateRefitPFO::CreateRefitPFO():marlin::Processor("CreateRefitPFO"){
    _description = "CreateRefitPFO creates new RECONSTRUCTEDPARTICLE collection that PFOs are updated using tracks refitted with true mass for protons and kaons";
}

void CreateRefitPFO::init(){
}

void CreateRefitPFO::processEvent( EVENT::LCEvent* event ){
	LCCollection* pfos = event->getCollection("PandoraPFOs");
    LCCollection* tracksCol = event->getCollection("MarlinTrkTracks");
    // LCCollection* tracksKaonRefitCol = event->getCollection("MarlinTrkTracks");
    // LCCollection* tracksProtonRefitCol = event->getCollection("MarlinTrkTracks");
	LCCollection* tracksKaonRefitCol = event->getCollection("MarlinTrkTracksKaon");
	LCCollection* tracksProtonRefitCol = event->getCollection("MarlinTrkTracksProton");
    LCRelationNavigator trackToMcNav( event->getCollection("MarlinTrkTracksMCTruthLink") );

	LCCollectionVec* outputPfosCol = new LCCollectionVec(LCIO::RECONSTRUCTEDPARTICLE);

	for(int i=0; i<pfos->getNumberOfElements(); ++i){
		ReconstructedParticle* pfo = static_cast<ReconstructedParticle*>( pfos->getElementAt(i) );
        const std::vector<Track*>& tracks = pfo->getTracks();
        int nTracks = tracks.size();

        //Substitute ALL Kaon/Proton tracks inside this PFO
        std::vector<Track*> refittedTracks;
        std::vector<int> tracksPdg;
        for(int j=0; j<nTracks; ++j){
            Track* track = tracks[j];
            int pdg = getTrackPDG(track, trackToMcNav);
            int idx = getTrackIndex(tracksCol, track);
            if ( std::abs(pdg) == 321 ) track = static_cast<Track*> ( tracksKaonRefitCol->getElementAt( idx ) );
            else if( std::abs(pdg) == 2212 ) track = static_cast<Track*> ( tracksProtonRefitCol->getElementAt( idx ) );
            tracksPdg.push_back(pdg);
            refittedTracks.push_back(track);
        }

        // create new PFO substituting tracks. Everything else just copy from the old pfo
        ReconstructedParticleImpl* outputPFO = new ReconstructedParticleImpl();
        for(size_t j=0; j<pfo->getTracks().size(); ++j) outputPFO->addTrack(refittedTracks[j]);

        outputPFO->setType(pfo->getType());
        outputPFO->setMomentum(pfo->getMomentum());
        outputPFO->setEnergy(pfo->getEnergy());
        outputPFO->setCovMatrix(pfo->getCovMatrix());
        outputPFO->setMass(pfo->getMass());
        outputPFO->setCharge(pfo->getCharge());
        outputPFO->setReferencePoint(pfo->getReferencePoint());
        outputPFO->setParticleIDUsed(pfo->getParticleIDUsed());
        outputPFO->setGoodnessOfPID(pfo->getGoodnessOfPID());
        outputPFO->setStartVertex(pfo->getStartVertex());
        for(size_t j=0; j<pfo->getParticles().size(); ++j) outputPFO->addParticle(pfo->getParticles()[j]);
        for(size_t j=0; j<pfo->getClusters().size(); ++j) outputPFO->addCluster(pfo->getClusters()[j]);
        for(size_t j=0; j<pfo->getParticleIDs().size(); ++j){
            ParticleIDImpl* inPID = static_cast<ParticleIDImpl*>( pfo->getParticleIDs()[j] );
            ParticleIDImpl* outPID = new ParticleIDImpl;
            outPID->setType(inPID->getType());
            outPID->setPDG(inPID->getPDG());
            outPID->setLikelihood(inPID->getLikelihood());
            outPID->setAlgorithmType(inPID->getAlgorithmType()) ;
            for(size_t k=0; k<inPID->getParameters().size(); ++k) outPID->addParameter( inPID->getParameters()[k] );
            outputPFO->addParticleID( outPID );
        }
        outputPfosCol->addElement( outputPFO );
    }
    event->addCollection( outputPfosCol , "updatedPandoraPFOs" );
}

int CreateRefitPFO::getTrackIndex(EVENT::LCCollection* trackCollection, EVENT::Track* selectedTrack){
	for (int i=0; i<trackCollection->getNumberOfElements(); ++i){
		Track* track = static_cast<Track*>( trackCollection->getElementAt(i) );
		if ( track == selectedTrack ) return i;
    }
    return -1;
}

int CreateRefitPFO::getTrackPDG(EVENT::Track* track, UTIL::LCRelationNavigator& nav){
    const std::vector<LCObject*>& particles = nav.getRelatedToObjects(track);
    const std::vector<float>&  weights = nav.getRelatedToWeights(track);
    if( particles.size() == 0 ) return 0;

    int i = std::max_element( weights.begin(), weights.end() ) - weights.begin();
	MCParticle* mc = static_cast<MCParticle*> (particles[i]);
    return mc->getPDG();
}
