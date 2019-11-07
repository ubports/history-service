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
- Import android exported voice and text events `tools/backup` [1]

[1] Using https://github.com/handschuh/Slight-backup on the android side to
export.

### Backup

Copy & backup first your existing `history.sqlite` from your phone to your desktop. We
are here assuming `account0` that is the first sim slot on your phone.

```
$ adb pull /home/phablet/.local/share/history-service/history.sqlite ~/.local/share/history-service/
$ cd tools/backup
$ cmake && make
$ ./import-backup ofono/ofono/account0 calllogs_export.xml messages_export.xml
$ adb push ~/.local/share/history-service/history.sqlite /home/phablet/.local/share/history-service/
```
