mod_detect_overload
===================

This module is currently in proof-of-concept state.

A apache module which discovers overload situations.
In some situations administrators want to prefer certain users of their systems depending on the load situation 
of the website. Performance problems in components (webserver, database server, shared filesystem, external webservices, ...)
of web applications always result in a increasing number of busy workers. 

This module calculates the percentage of busy apache workers in relation to the total number 
of available workers. If a given percentage value is exceeded this module sets the environment variable "OVERLOAD" 
to "yes". This environment value can be used by rewrite rules or application code to handle the situation.

A typical usecase is e-commerce system which is overloaded because of a advertisment. With this module it is possible 
to forward users which do not actually own a valid session to a parking page which informs over the overload situation 
and automatically redirects the user after a few seconds to the desired page.
Users with a session (i.e. a shoppingcart) can finish their purchase without having long wait times.


## Compile and Install
-------------------

Prerequisites:
 * C-compiler
 * APXS

On Ubuntu just do:
```
git clone git://github.com/scoopex/mod_detect_overload.git mod_detect_overload
cd mod_detect_overload
sudo apt-get install apache2-prefork-dev # alternatively: apache2-threaded-dev
apxs -c mod_detect_overload.c
sudo apxs -i -a -n mod_detect_overload mod_detect_overload.la
```

Usage
-----

Modify apache configuration:
```
# Load Module
LoadModule detect_overload_module modules/mod_detect_overload.so
# Enable Module (TODO)
EnableOverloadDetection On
# Configure busy percentage limit
BusyPercentageLimit 90

RewriteEngine on
RewriteCond %{ENV:OVERLOAD} yes
RewriteCond %{REQUEST_URI} !=/maintenance.htm
# RewriteCond %{REQUEST_URI} !^/shoppingcart/.*
RewriteRule ^(.*)$ /maintenance.htm [R=301,L]
```

Testing
-----

 * Configure apache to log the modules behavior
 
```
# Configure apache log for detailed information
# (adds overload state in the last column)
LogFormat "%h %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-Agent}i\" %{OVERLOAD}e %{OVERLOAD_INFO}e" combined_marc
CustomLog "logs/access_log" combined_marc
```
 * Configure apache workers to handle a highe amount of requests

```
StartServers          2
MaxClients            10
MinSpareThreads       5
MaxSpareThreads       6
ThreadsPerChild       10
MaxRequestsPerChild   1000000
```
 * Run apache in debug mode
   ```
   httpd -X
   ```
 * Configure a load test to simulate a normal load situation
   ```
   ab -c 7 -n 10000000 http://127.0.0.1
   ```
 * Configure a load test to simulate a overload situation
   ```
   ab -c 7 -n 10000000 http://127.0.0.1
   ```

Open issues
----------------
- Implement configuration switches "EnableOverloadDetection" and "BusyPercentageLimit"
- Implement a status page
- Enable module on location/directory basis
- Make code more modular and beautiful (forgive me, this is a proof-of-concept)

Licence and Authors
-------------------

Additional authors are very welcome - just submit your patches as pull requests.

 * Marc Schoechlin <ms@256bit.org>

License - see: LICENSE.txt
