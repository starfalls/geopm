#!/bin/bash
#
#  Copyright (c) 2015 - 2023, Intel Corporation
#  SPDX-License-Identifier: BSD-3-Clause
#

# This script sets up GEOPM configuration files to enforce an average power cap
# for all jobs of 50 watts less than TDP.  Jobs that opt into using GEOPM
# will be required to use the power_balancer agent, which will allow them
# to achieve better performance if their apps have been annotated with
# GEOPM epoch markup.
# This script should be run on every compute node for which this configuration is desired.
#
# This script modifies system files and typically requires root
# because these files are not expected to be writeable by non-admin users.
#

set -x

# Calculate the desired fixed power cap
POWER_CAP=$(($(geopmread CPU_POWER_LIMIT_DEFAULT board 0)-50))

# Remove any existing configuration
rm -rf /etc/geopm

# Create the policy file in a location accessible to the compute node
mkdir -p /etc/geopm
POLICY_FILE_PATH="/etc/geopm/power_balancer_policy.json"
geopmagent -a power_balancer -p $POWER_CAP > $POLICY_FILE_PATH
# This file should look similar to:
#   {
#     "CPU_POWER_LIMIT": 230
#   }
# for a system where the TDP is 140 W per package or 280 W total.

# Set the GEOPM configuration to use this policy file and the
# power balancer agent.
echo "{\"GEOPM_AGENT\": \"power_balancer\", \"GEOPM_POLICY\": \"$POLICY_FILE_PATH\"}" > $(geopmadmin --config-override)

# This file should look similar to the following:
# {
#    "GEOPM_AGENT": "power_balancer",
#    "GEOPM_POLICY": "/etc/geopm/power_balancer_policy.json"
# }

# Example sanity checks of the configuration
#
# Non-GEOPM jobs are power limited
#   > srun geopmread MSR::PKG_POWER_LIMIT:PL1_POWER_LIMIT package 0
#   115
#
# GEOPM jobs use the balancer and 230 W limit policy
#   > geopmlaunch srun -N1 -n1 --geopm-report=plugin_test.report -- geopmbench ~/short.conf > geopm_stdout 2>&1 && grep 'Policy\|Agent' plugin_test.report
#   Agent: power_balancer
#   Policy: {"CPU_POWER_LIMIT": 230}
#
# GEOPM jobs cannot use a different agent
#   > geopmlaunch srun -N1 -n1 --geopm-agent=monitor --geopm-report=plugin_test.report -- geopmbench ~/short.conf > geopm_stdout 2>&1 && grep 'Policy\|Agent' plugin_test.report
#   Agent: power_balancer
#   Policy: {"CPU_POWER_LIMIT": 230}
#   > grep Warning geopm_stdout
#   Warning: <geopm> User provided environment variable "GEOPM_AGENT" with value <monitor> has been overridden with value <power_balancer>
