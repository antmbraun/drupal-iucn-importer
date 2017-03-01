Drupal IUCN importer
====================================

This module allows Drupal users to check animal scientific names and conservation statuses 
against the IUCN API and provides a seamless way to update their data to 
be in sync with IUCN.



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

