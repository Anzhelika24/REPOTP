#!/bin/bash

git clone -b main git@github.com:Anzhelika24/REPOTP.git

# shellcheck disable=SC2164
cd REPOTP/python_proj

pip install -r requirements.txt

chmod +x main.py
