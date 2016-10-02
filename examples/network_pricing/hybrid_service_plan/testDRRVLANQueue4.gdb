set breakpoint pending on
exec-file ../../../../omnetpp-4.6/bin/opp_run
## for Windows (separator is ';')
set args -l ../../../src/inet -n ../../../examples;../../../src -u Cmdenv -f testStackedVlans.ini -c hybrid-test -r 0
# ## for Linux (separator is ':')
# set args -l ../../../src/inet -n ../../../examples:../../../src -u Cmdenv -f testStackedVlans.ini -c hybrid-test -r 0
tbreak DRRVLANQueue4::initialize
tbreak DRRVLANQueue4::handleMessage
tbreak DRRVLANQueue3.cc:88
tbreak DRRVLANQueue::initialize
# break VLANTagger2.cc:218
# display messageLength
# display payloadLength
# display numSent
# display currentTime
run
