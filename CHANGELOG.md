## New Features

- **_Message Fadeout_** setting
- **_Weapon Bob Speed_** setting
- **_[Color] Contrast_** setting [by @pvictress]

## Changes

- Nugget's translucency features now use translucency maps from a shared pool,
  potentially improving program startup time in exchange for stutters
  when enabling said features for the first time since launch

## Bug Fixes

- Broken movement in systems assuming `char`s to be unsigned
- `comp_lsamnesia` inversely affecting collision with entities (i.e. lost souls would remember their target)
- Message grouping only checking as many characters as the last message had,
  causing incorrect grouping (e.g. new message "TEST" being grouped with last message "TES")
