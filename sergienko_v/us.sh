#!/bin/bash
getent passwd | cut -d: -f1,3,4 > us.txt

