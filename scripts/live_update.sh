#!/usr/bin/env bash
echo "Sending $4 to tenant $2 at $1"
curl -H "X-PostKey: $3" -H "Host: $2" --data-binary "@$4" -X POST $1
