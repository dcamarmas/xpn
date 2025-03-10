# xpn_server_monitor

The xpn_server_monitor tool is responsible for collecting the aggregate data statistics of the servers in a csv file.

### This tool requieres to set two enviromental variables:
```bash
# Flag to activate statistical data collection
export XPN_STATS=1
# Variable with a existing path to the directory that will store the data
export XPN_STATS_DIR=<path to the directory that will store the data>
# Variable with the path to the config file
export XPN_CONFIG=<path to the config file>
```

## An example of use is as follows:
```bash
# Flag to activate/desactivate (optional)
DO_STATS=0

if [[ "$DO_STATS" -eq 1 ]]; then
    # Set the env flags
    # Create the necesary directories
    export XPN_STATS=1
    export XPN_STATS_DIR=<path to the directory that will store the data>

    mkdir -p $XPN_STATS_DIR
fi

############################################
# Create configuration and start the servers
############################################

if [[ "$DO_STATS" -eq 1 ]]; then
    # Start the monitor, it need to run in the background
    xpn_server_monitor &
fi

######################
# Run the applications
######################

if [[ "$DO_STATS" -eq 1 ]]; then
    # Stop the monitor
    xpn_server_monitor stop
fi

##################
# Stop the servers
##################
```


