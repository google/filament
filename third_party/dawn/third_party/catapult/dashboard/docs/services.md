# Chromeperf Services

This dashboard organizes groups of functionality into a few services:

 - `default` handles all requests not explicitly handled by another service.
 - `pinpoint` handles all frontend and backend work for Pinpoint. Handlers are
   all threadsafe.
 - `upload` handles all work to accept new data from uploaders and add it to the
   datastore. Clients are generally scripts running on a bot. Expected load is
   about 100QPS. Handlers are not yet threadsafe.
 - `api` handles /api/ requests to serve data from the datastore to clients such
   as the new SPA frontend and Soundwave. Expected load is about 1QPS with
   bursts up to a few dozen concurrent requests, with relatively smaller memory
   requirements. No taskqueues use this service, so all requests must return
   within 60s. Generally, SWEs wait for these requests interactively. Both
   Soundwave and the SPA cache results in the client. Handlers are all
   threadsafe.
