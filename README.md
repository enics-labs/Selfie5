# Selfie5
Self-contained verification utility for high-throughput random testing of  processors with RISC-V adaptation  

This project provide the source code for the utility to be presented in the DATE Conference (Design, Automation and Test in Europe) 25-27 March 2024  

by following paper:  

*Selfie5: an autonomous, self-contained verification approach for high-throughput random testing of programmable processors*  
Link to be provided upon publication release.   

__Currently setup and running guidelines are under WORK IN PROGRESS, to be accomplished by publication release at towards the above conference.__

# Download and initial installation

```bash
# Open a bash terminal at your desired Installation space.
git clone git@github.com:enics-labs/Selfie5.git ;# Clone the repository
cd Selfie5/runspace ; # Go to Runspace
../scripts/comp_rvsim_no_dut.sh  ;# Compile in RVSIM non-dut mode
./selfie5.exe ;# Execute
```
  
# Running from Google Colab

Simply open a Goole Colab Notebook, create a code cell, copy paste below lines and run the cell  


 ```bash
%%shell
cd /Content
\rm -rf Selfie5 ;# Just in case it exist from previous experiment
git clone https://github.com/enics-labs/Selfie5.git
cd Selfie5/runspace
chmod u+x ../scripts/comp_rvsim_no_dut.sh 
../scripts/comp_rvsim_no_dut.sh
./selfie5.exe ;# Execute
```


