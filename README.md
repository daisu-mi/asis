# asis
## what is this?
* It looks-up an AS number from the specified IP address
* It acts like "WHOIS" service, but is lightweight.
## setup
* `% configure`
* `% make`
* `% sudo make install`
## run
* `% sudo asis -c routerview.pfx2as`
## then you can lookup an AS number with your whois client
* `% whois -h localhost 127.0.0.1`
## If you want to lookup the realistic AS Number, you can use CAIDA's routerview.pfx2as.
### see http://data.caida.org/datasets/routing/routeviews-prefix2as
* `aslookup -b 127.0.0.1 -c your_downloaded/routerviews-prefix2as`

