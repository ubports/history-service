# History service

The history-service is used to store voice, text events into a database.
Applications using it are `messaging-app` and the `dialer-app`.

Currently a `sqlite` db is used. The path to the db file is `~/.local/share/history-service/history.sqlite`.

# Building

Specify the install prefix or it won't find some files. Look into
`snapcraft.yaml` for dependencies.

```
$ cmake -DCMAKE_INSTALL_PREFIX=/usr
$ make
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
