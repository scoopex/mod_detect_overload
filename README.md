mod_detect_overload
===================

This module is currently in proof-of-concept state.

A apache module which discovers overload situations.
In some situations administrators want to prefer certain users of their systems depending on the load situation of the website.
This module calculates the percentage of busy apache workers in relation to the total number of available workers. If a given percentage value is execeeded
this module sets the environment variable "OVERLOAD" to "yes". This environment value can be used by rewite rules or application code to handle the situation.

A typical usecase is e-commerce system which is overloaded because of a advertisment. With this module it is possible to forward users which do not actually 
own a valid session to a parking page which informs over the overload situation.
Users with a session (i.e. a shoppingcart) can finish their purchase.


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
# Configure apache log for detailed information
# (adds overload state in the last column)
LogFormat "%h %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-Agent}i\" %{OVERLOAD}e %{OVERLOAD_INFO}e" combined_marc
CustomLog "logs/access_log" combined_marc

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
