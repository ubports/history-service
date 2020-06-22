# History service

Service that provides call log and conversation history.

## Getting Started


History service provide the database and an API to store/retrieve the call log (used by dialer-app ) and the sms/mms history ( used by messaging-app ).

See as well telepathy-ofono for incoming message events.

Database location: `~.local/share/history-service/history.sqlite`


### Installing

You can use [crossbuilder](http://docs.ubports.com/en/latest/systemdev/testing-locally.html#cross-building-with-crossbuilder)
( you may need to add manually dh-translations, e.g `crossbuilder inst-foreign dh-translations)

```
crossbuilder
```

## Examples

Under `tools` are some examples:

- Add text events `tools/maketextevents`
- Add voice events `tools/makevoiceevents`
- Convenient manager to access the entries `tools/reader`
- Restore Android voice calls and text history `tools/backup`

### Restore Android voice calls and text history

On Android download slightbackup from [fdroid](https://f-droid.org/en/packages/de.shandschuh.slightbackup/)
or use the source code from [github](https://github.com/handschuh/Slight-backup).
The backups (xml files) are stored unencrypted in an editable location on the SD card.
Copy these xml files to your desktop. We assume here the names
`calllogs_example.xml` and `messages_example.xml`

On ubuntu phone copy & backup your existing `history.sqlite` to your desktop. We
are assuming `account0` that is the first sim slot on your phone.

Now run the commands:

```
$ adb pull /home/phablet/.local/share/history-service/history.sqlite ~/.local/share/history-service/
$ cd tools/backup
$ cmake && make
$ ./import-backup ofono/ofono/account0 calllogs_example.xml messages_example.xml
$ adb push ~/.local/share/history-service/history.sqlite /home/phablet/.local/share/history-service/
```

## Running the tests

Run tests within the container `crossbuilder shell` and find the generated tests, currently on `history-service/obj-..../tests/`


## Contributing

Please read [CONTRIBUTING.md](http://docs.ubports.com/en/latest/systemdev/testing-locally.html).


## License

GPL v3.0 - see the [COPYING](COPYING) file for details
