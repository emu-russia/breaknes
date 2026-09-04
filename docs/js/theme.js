/* Breaknes — shared dark/light theme toggle.
   Reads the initial theme from localStorage('breaknes-theme'),
   falls back to the system preference, then keeps the choice.
   Button labels are taken from data-label-light / data-label-dark
   so the same script works for every page language. */
(function () {
  'use strict';
  var root = document.documentElement;
  var KEY = 'breaknes-theme';
  var saved = null;
  try { saved = window.localStorage.getItem(KEY); } catch (e) {}
  var dark = saved !== null
    ? saved === 'dark'
    : !!(window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches);

  function apply(isDark) {
    root.setAttribute('data-theme', isDark ? 'dark' : 'light');
    var btn = document.getElementById('themeToggle');
    if (!btn) return;
    var attr = isDark ? 'data-label-dark' : 'data-label-light';
    var label = btn.getAttribute(attr) || 'Toggle theme';
    btn.setAttribute('aria-label', label);
    btn.setAttribute('title', label);
  }

  apply(dark);

  var btn = document.getElementById('themeToggle');
  if (btn) {
    btn.addEventListener('click', function () {
      dark = root.getAttribute('data-theme') !== 'dark';
      apply(dark);
      try { window.localStorage.setItem(KEY, dark ? 'dark' : 'light'); } catch (e) {}
    });
  }
})();
