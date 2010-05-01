<?php
// $Id: uc_wishlist.admin.inc,v 1.1 2009/03/27 16:08:33 rszrama Exp $

/**
 * @file
 * Admin page callbacks and forms for wish lists.
 */


// Build the admin settings form.
function uc_wishlist_admin_settings_form() {
  $form = array();

  $form['uc_wishlist_default_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Default wish list title'),
    '#default_value' => variable_get('uc_wishlist_default_title', "%user's wish list"),
    '#description' => t("The default name of a new wish list. The token %user will be replaced by the user's name."),
  );
  $form['uc_wishlist_allow_anonymous'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow anonymous users to create wish lists.'),
    '#default_value' => variable_get('uc_wishlist_allow_anonymous', FALSE),
    '#description' => t('Check this box to allow anonymous users to create wish lists. If not checked, anonymous visitors will be prompted to login or register before creating a wish list.'),
  );

  // TODO: add uc_wishlist_default_length

  return system_settings_form($form);
}

// View and manage the wish lists on the site.
function uc_wishlist_admin() {
  $rows = array();

  $header = array(
    array('data' => t('User'), 'field' => 'u.name', 'sort' => 'asc'),
    array('data' => t('Title'), 'field' => 'w.title'),
    array('data' => t('Expiration date'), 'field' => 'w.expiration'),
    array('data' => t('Status'))
  );

  // Get a paged list of wish lists from the database.
  $result = pager_query("SELECT w.wid, w.uid, w.title, w.expiration, u.name FROM {uc_wishlists} AS w LEFT JOIN {users} AS u ON w.uid = u.uid". tablesort_sql($header), 25);

  while ($wishlist = db_fetch_object($result)) {
    // Build the operations array for the wish list.
    $op = array(
      $wishlist->expiration < time() ? t('Expired') : t('Active'),
      l(t('Delete'), 'admin/store/customers/wishlist/'. $wishlist->wid .'/delete'),
    );

    $rows[] = array(
      $wishlist->name ? l(check_plain($wishlist->name), 'user/'. $wishlist->uid) : t('Anonymous'),
      l(filter_xss($wishlist->title), 'wishlist/'. $wishlist->wid),
      format_date($wishlist->expiration),
      implode(' | ', $op),
    );
  }

  if (empty($rows)) {
    $rows[] = array(array('data' => t('No wish lists found.'), 'colspan' => 4));
  }

  return theme('table', $header, $rows) . theme('pager');
}

// Confirm the deletion of a wish list.
function uc_wishlist_admin_delete_form($form_state, $wishlist) {
  $form = array();

  $form['wishlist'] = array(
    '#type' => 'value',
    '#value' => $wishlist,
  );

  return confirm_form($form, t('Are you sure you want to delete wish list %title?', array('%title' => $wishlist->title)), 'admin/store/customers/wishlist', NULL, t('Delete'));
}

function uc_wishlist_admin_delete_form_submit($form, &$form_state) {
  // Delete the wish list.
  uc_wishlist_delete($form_state['values']['wishlist']->wid);

  // Display a message and redirect back to the admin table.
  drupal_set_message(t('Wish list %title deleted.', array('%title' => $form_state['values']['wishlist']->title)));

  $form_state['redirect'] = 'admin/store/customers/wishlist';
}

