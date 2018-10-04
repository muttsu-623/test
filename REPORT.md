# Report

## Description

This is my reaech report. I write what I did and what I will do.

## Daily

### Final
- [ ] dicover why reconstraction is failed.

### 20181009
- [ ] Understand Okugawa-san's research and know why reconstraction is failed.
- [ ] Display other detectors.
- [ ] Explain particles in a display.

### 20181004
- [x] Discover why are tracks drawn only until they reaches detector.
    - I discovered it. In TEveTrackPropagator class, there are MaxZ and MaxR properties, and they are set generally(2000, 4000). So I changed them using SetMaxZ and SetMaxR methods.

### 20181003
- [ ] Discover why are tracks drawn only until they reaches detector.
    - [ ] I couldn't discover it.

### 20180927
- [x] Display TCP detector.
- [x] Display other particles in the ROOT file
    - [x] myE347-Ptt-ln4q.eL.pR.01_4j.root

### 20180926
- [x] I write a tube and xyz axis on the event display.
- [x] Don't reset camera when changing particles momentums.
- [ ] ~~Display other particles in the ROOT file~~
    - [ ] ~~myE347-Ptt-ln4q.eL.pR.01_4j.root~~
- [ ] ~~Display detectors.~~
### 20180925
* I read "dev20180921" branch code to understand what is happen on the eventdisplay.
    * "dev 20180921" branch is created by Mr. Yonamine.