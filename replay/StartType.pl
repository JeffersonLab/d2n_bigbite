#!/bin/bash
#
# This is a simple wrapper that calls the associated script in $DB_DIR
# -- Brad Sawatzky

exec $DB_DIR/StartType.pl "$@"
