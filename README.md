# history-service

Service that provides call log and conversation history

## Getting Started


History service provide the database and an API to store/retrieve the call log (used by dialer-app ) and the sms/mms history ( used by messaging-app ).

See as well telepathy-ofono for incoming message events.

Database location: ~.local/share/history-service/history.sqlite


### Installing

You can use [crossbuilder](http://docs.ubports.com/en/latest/systemdev/testing-locally.html#cross-building-with-crossbuilder)
( you may need to add manually dh-translations, e.g `crossbuilder inst-foreign dh-translations)

```
crossbuilder
```

## Running the tests

Run tests within the container `crossbuilder shell` and find the generated tests, currently on `history-service/obj-..../tests/`


## Contributing

Please read [CONTRIBUTING.md](http://docs.ubports.com/en/latest/systemdev/testing-locally.html).


## License

GPL v3.0 - see the [COPYING](COPYING) file for details

