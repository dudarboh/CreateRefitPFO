# CreateRefitPFO

This is a copy of Yasser's processor
https://github.com/yradkhorrami/ChargedPFOCorrection/tree/master

with a code clean-up, removed all histogram output, retaining only core logic of what processor supposed to do:

1. Create a new "updatedPandoraPFOs" collection which will assume perfect pID of kaons and protons.
2. Substitute original PFO tracks from "MarlinTrkTracks" collection to tracks from "MarlinTrkTracks" (pions/other), "MarlinTrkTracksKaon" (kaons), "MarlinTrkTracksProton" collections based on their PDG.
3. Update PFO invariant mass/energy/covariance matrix parameters respectively.
