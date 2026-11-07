// The scripts inline in the two pages, read by eslint-plugin-html, which hands eslint each
// <script> of a page as if it were a file of its own. The recommended rules and nothing else to
// begin with, as .clang-tidy began: what they report is a mistake rather than a matter of taste.
import js from '@eslint/js';
import html from 'eslint-plugin-html';
import globals from 'globals';

export default [
    {
        ...js.configs.recommended,
        files: ['web/**/*.html'],
        plugins: { html },
        languageOptions: {
            ecmaVersion: 'latest',
            sourceType: 'script',
            globals: globals.browser,
        },
    },
];
