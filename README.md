Drupal IUCN importer
====================================

This module allows Drupal users to check animal scientific names and conservation statuses 
against IUCN data using their API. It also provides a seamless way to update their site's data.

### Installation
  1. Install like any other drupal module
  2. Go to /admin/content/iucn/settings
    1. Enter your IUCN API token. You can obtain one here: http://apiv3.iucnredlist.org/api/v3/token
    2. Select your "Animal" content type. It must contain a field for scientific name and a field for IUCN conservation status.
    3. Select your scientific name and IUCN conservation status fields.
    4. Click save.
    
### Usage
  1. Click the "Check IUCN Data" tab.
  2. Click the "Pull new data from IUCN" button.
  3. A table should be generated listing inconsistencies between your animal data and IUCN's data.
  4. This table will be kept up-to-date with any changes that occur to your animal nodes, e.g. if someone changes the value of the IUCN conservation field on a particular node, this module will check to see whether the value conforms with IUCN and if not, will add a row to this table.
  5. The links to update your data to match IUCN data are self-explanatory.

