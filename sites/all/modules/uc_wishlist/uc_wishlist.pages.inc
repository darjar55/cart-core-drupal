<?php
// $Id: uc_wishlist.pages.inc,v 1.1.2.2 2010/02/22 15:46:04 longwave Exp $

/**
 * @file
 * Page callback and functions for wish lists.
 */


/**
 * Display a wish list for viewing or altering.
 *
 * @param $wid
 *   The ID of the wish list to display.
 * @return
 *   HTML output for the wish list display and form.
 */
function uc_wishlist_display($wid = NULL) {
  global $user;
  $output = '';

  // $own defines whether the wish list is owned by the current user or not.
  // This affects how many parts of the page are rendered.
  $own = FALSE;

  // Default to the current user's wish list if no wishlist ID is specified.
  if (empty($wid)) {
    $wid = uc_wishlist_get_wid();
    $own = TRUE;
  }
  elseif ($wid == uc_wishlist_get_wid()) {
    $own = TRUE;
  }

  // Attempt to load the wish list.
  $wishlist = uc_wishlist_load($wid);

  // Handle a non-existent wish list.
  if (!$wishlist) {
    // If the requested list was for the current user...
    if ($own) {
      // Display a message letting them know their list is empty.
      drupal_set_title(t('Wish list'));
      drupal_set_message(t("You have not added any products to your wish list. You can add any product from the store to your wish list by clicking the 'Add to wish list' button on the product's page."));

      return t('There are no products on this wish list.');
    }
    else {
      // Otherwise send them to the search form.
      drupal_set_message(t('The wish list you requested could not be found.  Perhaps you can try looking for it through the wish list search form below.'));
      drupal_goto('wishlist/search');
    }
  }

  drupal_set_title(check_plain($wishlist->title));

  // Add the settings form if the user is viewing his own wish list.
  if ($own) {
    if (!$user->uid) {
      drupal_set_message(t('You must <a href="!login_url">login</a> or <a href="!register_url">register</a> to save your wish list.', array('!login_url' => url('user/login'), '!register_url' => url('user/register'))));
    }

    $collapsed = TRUE;

    if (empty($wishlist->address->firstname) || empty($wishlist->address->lastname) || empty($wishlist->address->addr1) || empty($wishlist->address->city) || empty($wishlist->address->postcode)) {
      $collapsed = FALSE;
    }

    $output .= drupal_get_form('uc_wishlist_settings_form', $wishlist, $collapsed);
  }

  // Add expiration information to the display.
  if ($wishlist->expiration < time()) {
    $output .= '<p>'. t('This wish list may be no longer valid. It was for an event on @date.', array('@date' => date('F j, Y', $wishlist->expiration))) .'</p>';
  }
  elseif ($wishlist->expiration > 0) {
    $output .= '<p>'. t('This wish list is valid until @date.', array('@date' => date('F j, Y', $wishlist->expiration))) .'</p>';
  }

  $items = uc_wishlist_get_contents($wid);

  if (empty($items)) {
    return '<p>'. t('There are no products on this wish list.') .'</p>';
  }

  $output .= drupal_get_form('uc_wishlist_view_form', $items, $wid, $own);

  return $output;
}

// Build the user wish list settings form.
function uc_wishlist_settings_form($form_state, $wishlist, $collapsed = FALSE) {
  global $user;

  drupal_add_js(drupal_get_path('module', 'uc_wishlist') .'/uc_wishlist.js');

  $expiration = getdate($wishlist->expiration);
  $expiration['day'] = $expiration['mday'];
  $expiration['month'] = $expiration['mon'];

  $form = array();

  $form['wishlist'] = array(
    '#type' => 'fieldset',
    '#title' => t('Wish list settings'),
    '#collapsible' => TRUE,
    '#collapsed' => $collapsed,
  );
  $form['wishlist']['wid'] = array(
    '#type' => 'hidden',
    '#value' => $wishlist->wid,
  );
  $form['wishlist']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $wishlist->title,
    '#required' => TRUE,
  );
  $form['wishlist']['expiration'] = array(
    '#type' => 'date',
    '#title' => t('Event or expiration date'),
    '#default_value' => $expiration,
    '#description' => t('If this wish list is associated with an event or will no longer be relevant on a specific date, enter it here.'),
  );

  $form['wishlist']['address'] = array(
    '#type' => 'fieldset',
    '#title' => t('Mailing address'),
    '#description' => t('The address you enter here will be available as a shipping address to anyone who purchases an item from your wish list.'),
  );

  if ($user->uid) {
    $addresses = uc_select_address($user->uid, 'delivery', 'apply_address(\'delivery\', this.value);', t('Saved addresses'), TRUE);
    if (!empty($addresses)) {
      $form['wishlist']['address']['delivery_address_select'] = $addresses;
      unset($form['wishlist']['address']['delivery_address_select']['#suffix']);
    }
  }

  if (uc_address_field_enabled('first_name')) {
    $form['wishlist']['address']['delivery_first_name'] = uc_textfield(uc_get_field_name('first_name'), $wishlist->address->firstname, uc_address_field_required('first_name'));
  }
  if (uc_address_field_enabled('last_name')) {
    $form['wishlist']['address']['delivery_last_name'] = uc_textfield(uc_get_field_name('last_name'), $wishlist->address->lastname, uc_address_field_required('last_name'));
  }
  if (uc_address_field_enabled('company')) {
    $form['wishlist']['address']['delivery_company'] = uc_textfield(uc_get_field_name('company'), $wishlist->address->company, uc_address_field_required('company'), NULL, 64);
  }
  if (uc_address_field_enabled('street1')) {
    $form['wishlist']['address']['delivery_street1'] = uc_textfield(uc_get_field_name('street1'), $wishlist->address->addr1, uc_address_field_required('street1'), NULL, 64);
  }
  if (uc_address_field_enabled('street2')) {
    $form['wishlist']['address']['delivery_street2'] = uc_textfield(uc_get_field_name('street2'), $wishlist->address->addr2, uc_address_field_required('street2'), NULL, 64);
  }
  if (uc_address_field_enabled('city')) {
    $form['wishlist']['address']['delivery_city'] = uc_textfield(uc_get_field_name('city'), $wishlist->address->city, uc_address_field_required('city'));
  }
  if (uc_address_field_enabled('country')) {
    $form['wishlist']['address']['delivery_country'] = uc_country_select(uc_get_field_name('country'), $wishlist->address->country, NULL, 'name', uc_address_field_required('country'));
  }
  if (uc_address_field_enabled('zone')) {
    if (isset($_POST['delivery_country'])) {
      $country_id = intval($_POST['delivery_country']);
    }
    else {
      $country_id = $wishlist->address->country;
    }
    $form['wishlist']['address']['delivery_zone'] = uc_zone_select(uc_get_field_name('zone'), $wishlist->address->zone, NULL, $country_id, 'name', uc_address_field_required('zone'));
    if (isset($_POST['panes']) && count($form['wishlist']['address']['delivery_zone']['#options']) == 1) {
      $form['wishlist']['address']['delivery_zone']['#required'] = FALSE;
    }
  }
  if (uc_address_field_enabled('postal_code')) {
    $form['wishlist']['address']['delivery_postal_code'] = uc_textfield(uc_get_field_name('postal_code'), $wishlist->address->postcode, uc_address_field_required('postal_code'), NULL, 10, 10);
  }
  if (uc_address_field_enabled('phone')) {
    $form['wishlist']['address']['delivery_phone'] = uc_textfield(uc_get_field_name('phone'), $wishlist->address->phone, uc_address_field_required('phone'), NULL, 32, 16);
  }

  $form['wishlist']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save settings'),
  );

  return $form;
}

function uc_wishlist_settings_form_validate($form, &$form_state) {
  global $user;

  $wid = $form_state['values']['wid'];

  $wishlist = uc_wishlist_load($wid);
  if (!$wishlist) {
    drupal_set_message(t('Could not find the specified wish list.'), 'error');
    return FALSE;
  }
  if ($wishlist->uid != $user->uid && !user_perm('administer store')) {
    drupal_set_message(t('You do not have permission to edit this wish list.'), 'error');
    return FALSE;
  }
}

function uc_wishlist_settings_form_submit($form, &$form_state) {
  $expiration = mktime(0, 0, 0, $form_state['values']['expiration']['month'], $form_state['values']['expiration']['day'], $form_state['values']['expiration']['year']);

  $address = array(
    'firstname' => $form_state['values']['delivery_first_name'],
    'lastname' => $form_state['values']['delivery_last_name'],
    'company' => $form_state['values']['delivery_company'],
    'addr1' => $form_state['values']['delivery_street1'],
    'addr2' => $form_state['values']['delivery_street2'],
    'city' => $form_state['values']['delivery_city'],
    'country' => $form_state['values']['delivery_country'],
    'zone' => $form_state['values']['delivery_zone'],
    'postcode' => $form_state['values']['delivery_postal_code'],
    'phone' => $form_state['values']['delivery_phone'],
  );

  uc_wishlist_update_wishlist($form_state['values']['wid'], $form_state['values']['title'], $expiration, (object) $address);

  drupal_set_message(t('Your wish list has been updated.'));
}

/**
 * Display a page allowing the customer to view his/her wish list.
 */
function uc_wishlist_view_form($form_state, $items, $wid, $own) {
  $form = array();

  $form['items'] = array(
    '#tree' => TRUE,
  );

  // Load each wish list product and add it to the form array.
  foreach ($items as $item) {
    $node = node_load($item->nid);
    $element = array();

    $element['nid'] = array('#type' => 'value', '#value' => $node->nid);
    $element['wpid'] = array('#type' => 'value', '#value' => $item->wpid);
    $element['module'] = array('#type' => 'value', '#value' => 'uc_product');

    if ($own) {
      $element['remove'] = array('#type' => 'checkbox');
    }

    $item->haveqty = 0;
    if (is_array($item->purchase)) {
      $item->haveqty = count($item->purchase);
    }

    $element['title'] = array(
      '#value' => l(filter_xss($node->title, array()), 'node/'. $node->nid),
    );

    $description = '';
    foreach (module_implements('cart_item_description') as $module) {
      $description .= module_invoke($module, 'cart_item_description', $item);
    }
    if ($description) {
      $element['description'] = array('#value' => $description);
    }

    $element['#total'] = $item->price * $item->qty;

    $element['data'] = array('#type' => 'hidden', '#value' => serialize($item->data));

    $element['wantqty'] = array(
      '#type' => 'textfield',
      '#default_value' => $item->qty,
      '#size' => 5,
      '#maxlength' => 6,
      '#disabled' => $own ? FALSE : TRUE
    );
    if (!$own) {
      // Disabled elements do not pass their default value.
      $element['wantqty']['#value'] = $item->qty;
    }
    $element['haveqty'] = array(
      '#type' => 'textfield',
      '#default_value' => $item->haveqty,
      '#size' => 5,
      '#maxlength' => 6,
      '#disabled' => TRUE
    );
    $element['qty'] = array(
      '#type' => 'textfield',
      '#default_value' => $item->qty - $item->haveqty > 0 ? $item->qty - $item->haveqty : 1,
      '#size' => 5,
      '#maxlength' => 6,
    );

    $element['addcart'] = array(
      '#type' => 'submit',
      '#name' => 'addcart-'. $item->wpid,
      '#value' => t('Add to cart'),
    );

    $form['items'][] = $element;
  }

  $form['wid'] = array(
    '#type' => 'hidden',
    '#value' => $wid,
  );

  if ($own) {
    $form['own'] = array(
      '#type' => 'value',
      '#value' => TRUE,
    );
    $form['update'] = array(
      '#type' => 'submit',
      '#value' => t('Update wish list'),
    );
  }

  return $form;
}

function theme_uc_wishlist_view_form($form) {
  $rows = array();

  $own = isset($form['own']) ? TRUE : FALSE;

  $header = array(t('Products'), '', t('Wanted'), t('Have'), t('Qty.'), t('Purchase'), array('data' => t('Total'), 'align' => 'right'));

  if ($own) {
    array_unshift($header, t('Remove'));
  }

  foreach (element_children($form['items']) as $i) {
    $nid = $form['items'][$i]['nid']['#value'];
    $desc = drupal_render($form['items'][$i]['title']) .'<br />';
    $desc .= drupal_render($form['items'][$i]['description']);

    $row = array(
      uc_product_get_picture($nid, 'cart'),
      $desc,
      $form['items'][$i]['wantqty'] ? drupal_render($form['items'][$i]['wantqty']) : '',
      $form['items'][$i]['haveqty'] ? drupal_render($form['items'][$i]['haveqty']) : '',
      $form['items'][$i]['qty'] ? drupal_render($form['items'][$i]['qty']) : '',
      $form['items'][$i]['addcart'] ? drupal_render($form['items'][$i]['addcart']) : '',
      array('data' => uc_currency_format($form['items'][$i]['#total']), 'nowrap' => 'nowrap'),
    );

    if (isset($form['items'][$i]['remove'])) {
      array_unshift($row, drupal_render($form['items'][$i]['remove']));
    }

    $rows[] = array('data' => $row, 'valign' => 'top');
  }

  $output = '<div id="wishlist-form-products">'. theme('table', $header, $rows) .'</div>';

  return $output . drupal_render($form);
}

function uc_wishlist_view_form_validate($form, &$form_state) {
  // Find wish list item associated with 'add to cart' submitted.
  foreach ($form_state['values'] as $key => $val) {
    if (strpos($key, 'addcart-') === 0) {
      $wpid = intval(substr($key, 8));
    }
  }

  // If adding item to cart, check quantities.
  if ($wpid) {
    foreach ($form_state['values']['items'] as $key => $item) {
      if ($item['wpid'] == $wpid && $item['qty'] == 0) {
        $item['qty'] = 1;
      }
      if ($item['qty'] && ($item['qty'] + $item['haveqty']) > $item['wantqty']) {
        form_set_error($key, t('You have selected to purchase more items than requested.'));
      }
    }
  }
}

function uc_wishlist_view_form_submit($form, &$form_state) {
  if (isset($form_state['values']['op']) && !empty($form_state['values']['op'])) {
    if ($form_state['values']['op'] == $form_state['values']['update']) {
      uc_wishlist_update_item_object((object) $form_state['values']);

      drupal_set_message(t('Your wish list has been updated.'));

      $form_state['redirect'] = 'wishlist/'. $form_state['values']['wid'];
    }
  }
  else {
    // Find wish list item associated with 'add to cart' submitted.
    foreach ($form_state['values'] as $key => $val) {
      if (strpos($key, 'addcart-') === 0) {
        $wpid = intval(substr($key, 8));
      }
    }

    if ($wpid) {
      foreach ($form_state['values']['items'] as $key => $item) {
        if ($item['wpid'] == $wpid) {
          if ($item['qty'] == 0) {
            $form_state['values']['items'][$key]['qty'] = 1;
          }
        }
        else {
          $form_state['values']['items'][$key]['qty'] = 0;
        }
      }

      uc_wishlist_add_items_to_cart((object) $form_state['values']);
      drupal_set_message(t('The item(s) have been added to your shopping cart.'));

      $form_state['redirect'] = 'cart';
    }
  }
}

// Display the wish list search form.
function uc_wishlist_search_form($form_state, $keywords = '') {
  global $user;
  $form = array();

  // Generate link to 'create or manage your wish list'.
  $path = 'wishlist';
  $query = NULL;

  if (!$user->uid && !variable_get('uc_wishlist_allow_anonymous', FALSE)) {
    $path = 'user';
    $query = 'destination=wishlist';
  }

  $form['wishlist_link'] = array(
    '#value' => '<div>'. l(t('Create or manage your wish list.'), $path, array('query' => $query)) .'</div>',
  );

  $form['search'] = array(
    '#type' => 'fieldset',
  );
  $form['search']['keywords'] = array(
    '#type' => 'textfield',
    '#title' => t('Search keywords'),
    '#description' => t('Enter the keywords to use to search wish list titles and addresses.'),
    '#default_value' => $keywords,
  );
  $form['search']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Search'),
  );

  if (!empty($keywords)) {
    // Check for user, wish list title, or address matches.
    $result = pager_query("SELECT DISTINCT w.wid, w.title FROM {uc_wishlists} AS w JOIN {users} AS u ON w.uid = u.uid WHERE u.name LIKE '%%%s%%' OR w.title LIKE '%%%s%%' OR w.address LIKE '%%firstname%%%s%%addr1%%' ORDER BY w.title", 25, 0, NULL, $keywords, $keywords, $keywords);
  }
  else {
    $result = pager_query("SELECT wid, title FROM {uc_wishlists} ORDER BY title", 25);
  }

  $links = array();

  while ($wishlist = db_fetch_object($result)) {
    $links[] = array(
      'title' => filter_xss($wishlist->title, array()),
      'href' => 'wishlist/'. $wishlist->wid,
    );
  }

  if (!empty($links)) {
    $output = theme_links($links, array('class' => 'wishlist'));
  }
  else {
    $output = t('No wish lists found.');
  }

  $form['output'] = array(
    '#value' => '<div><h2>'. t('Wish lists:') .'</h2>'. $output .'</div>',
  );

  return $form;
}

function uc_wishlist_search_form_submit($form, &$form_state) {
  if (empty($form_state['values']['keywords'])) {
    $form_state['redirect'] = 'wishlist/search';
  }
  else {
    $form_state['redirect'] = 'wishlist/search/'. drupal_urlencode($form_state['values']['keywords']);
  }
}

