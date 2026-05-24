# This directory will hold private headers for the server executable.
# Recommended structure:
#   include/server/net/        - TCP session, UDP game socket
#   include/server/sim/        - simulation tick, world state
#   include/server/db/         - database access layer (libpqxx)
#   include/server/metrics/    - Prometheus collectors
