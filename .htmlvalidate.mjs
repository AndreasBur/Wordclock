// The two pages under web/, checked as the whole documents they are: html-validate's recommended
// rules, and its document preset for what only a whole page can get wrong. The pages had no
// doctype until this file was added, which put both into quirks mode in every browser, and the
// recommended rules alone do not ask for one.
export default {
    extends: ['html-validate:recommended', 'html-validate:document'],
    rules: {
        // The pages leave out the tags HTML says may be left out - <head>, <body>, the closing
        // </html> - as Google's HTML style guide asks, the guide this repository's names follow
        // too. The parser puts them in; writing them would be bytes in the flash for nothing.
        'no-implicit-close': 'off',
    },
};
