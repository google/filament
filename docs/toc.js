// Populate the sidebar
//
// This is a script, and not included directly in the page, to control the total size of the book.
// The TOC contains an entry for each page, so if each page includes a copy of the TOC,
// the total size of the page becomes O(n**2).
class MDBookSidebarScrollbox extends HTMLElement {
    constructor() {
        super();
    }
    connectedCallback() {
        this.innerHTML = '<ol class="chapter"><li class="chapter-item expanded "><a href="dup/intro.html"><strong aria-hidden="true">1.</strong> Introduction</a></li><li><ol class="section"><li class="chapter-item expanded "><a href="dup/building.html"><strong aria-hidden="true">1.1.</strong> Build</a></li><li><ol class="section"><li class="chapter-item expanded "><a href="build/windows_android.html"><strong aria-hidden="true">1.1.1.</strong> Build for Android on Windows</a></li></ol></li><li class="chapter-item expanded "><a href="dup/contributing.html"><strong aria-hidden="true">1.2.</strong> Contribute</a></li><li class="chapter-item expanded "><a href="dup/code_style.html"><strong aria-hidden="true">1.3.</strong> Coding Style</a></li></ol></li><li class="chapter-item expanded "><a href="main/index.html"><strong aria-hidden="true">2.</strong> Core Concepts</a></li><li><ol class="section"><li class="chapter-item expanded "><a href="main/filament.html"><strong aria-hidden="true">2.1.</strong> Filament</a></li><li class="chapter-item expanded "><a href="main/materials.html"><strong aria-hidden="true">2.2.</strong> Materials</a></li></ol></li><li class="chapter-item expanded "><a href="samples/index.html"><strong aria-hidden="true">3.</strong> Tutorials and Samples</a></li><li><ol class="section"><li class="chapter-item expanded "><a href="samples/ios.html"><strong aria-hidden="true">3.1.</strong> iOS Tutorial</a></li><li class="chapter-item expanded "><a href="samples/web.html"><strong aria-hidden="true">3.2.</strong> Web Tutorial</a></li></ol></li><li class="chapter-item expanded "><a href="notes/index.html"><strong aria-hidden="true">4.</strong> Technical Notes</a></li><li><ol class="section"><li class="chapter-item expanded "><a href="notes/versioning.html"><strong aria-hidden="true">4.1.</strong> Versioning</a></li><li><ol class="section"><li class="chapter-item expanded "><a href="notes/branching.html"><strong aria-hidden="true">4.1.1.</strong> Branching</a></li><li class="chapter-item expanded "><a href="notes/release_guide.html"><strong aria-hidden="true">4.1.2.</strong> Release Guide</a></li></ol></li><li class="chapter-item expanded "><a href="dup/docs.html"><strong aria-hidden="true">4.2.</strong> Documentation</a></li><li class="chapter-item expanded "><a href="notes/debugging.html"><strong aria-hidden="true">4.3.</strong> Debugging</a></li><li><ol class="section"><li class="chapter-item expanded "><a href="notes/metal_debugging.html"><strong aria-hidden="true">4.3.1.</strong> Metal</a></li><li class="chapter-item expanded "><a href="notes/vulkan_debugging.html"><strong aria-hidden="true">4.3.2.</strong> Vulkan</a></li><li class="chapter-item expanded "><a href="notes/spirv_debugging.html"><strong aria-hidden="true">4.3.3.</strong> SPIR-V</a></li><li class="chapter-item expanded "><a href="notes/asan_ubsan.html"><strong aria-hidden="true">4.3.4.</strong> Running with ASAN and UBSAN</a></li><li class="chapter-item expanded "><a href="notes/instruments.html"><strong aria-hidden="true">4.3.5.</strong> Using Instruments on macOS</a></li><li class="chapter-item expanded "><a href="notes/coverage.html"><strong aria-hidden="true">4.3.6.</strong> Code coverage analysis</a></li></ol></li><li class="chapter-item expanded "><a href="notes/libs.html"><strong aria-hidden="true">4.4.</strong> Libraries</a></li><li><ol class="section"><li class="chapter-item expanded "><a href="dup/bluegl.html"><strong aria-hidden="true">4.4.1.</strong> bluegl</a></li><li class="chapter-item expanded "><a href="dup/bluevk.html"><strong aria-hidden="true">4.4.2.</strong> bluevk</a></li><li class="chapter-item expanded "><a href="dup/filamat.html"><strong aria-hidden="true">4.4.3.</strong> filamat</a></li><li class="chapter-item expanded "><a href="dup/gltfio.html"><strong aria-hidden="true">4.4.4.</strong> gltfio</a></li><li class="chapter-item expanded "><a href="dup/iblprefilter.html"><strong aria-hidden="true">4.4.5.</strong> iblprefilter</a></li><li class="chapter-item expanded "><a href="dup/matdbg.html"><strong aria-hidden="true">4.4.6.</strong> matdbg</a></li><li class="chapter-item expanded "><a href="dup/uberz.html"><strong aria-hidden="true">4.4.7.</strong> uberz</a></li></ol></li><li class="chapter-item expanded "><a href="notes/tools.html"><strong aria-hidden="true">4.5.</strong> Tools</a></li><li><ol class="section"><li class="chapter-item expanded "><a href="dup/beamsplitter.html"><strong aria-hidden="true">4.5.1.</strong> beamsplitter</a></li><li class="chapter-item expanded "><a href="dup/cmgen.html"><strong aria-hidden="true">4.5.2.</strong> cmgen</a></li><li class="chapter-item expanded "><a href="dup/cso_lut.html"><strong aria-hidden="true">4.5.3.</strong> cso-lut</a></li><li class="chapter-item expanded "><a href="dup/filamesh.html"><strong aria-hidden="true">4.5.4.</strong> filamesh</a></li><li class="chapter-item expanded "><a href="dup/normal_blending.html"><strong aria-hidden="true">4.5.5.</strong> normal-blending</a></li><li class="chapter-item expanded "><a href="dup/mipgen.html"><strong aria-hidden="true">4.5.6.</strong> mipgen</a></li><li class="chapter-item expanded "><a href="dup/matinfo.html"><strong aria-hidden="true">4.5.7.</strong> matinfo</a></li><li class="chapter-item expanded "><a href="dup/roughness_prefilter.html"><strong aria-hidden="true">4.5.8.</strong> roughness-prefilter</a></li><li class="chapter-item expanded "><a href="dup/specular_color.html"><strong aria-hidden="true">4.5.9.</strong> specular-color</a></li><li class="chapter-item expanded "><a href="dup/zbloat.html"><strong aria-hidden="true">4.5.10.</strong> zbloat</a></li></ol></li></ol></li></ol>';
        // Set the current, active page, and reveal it if it's hidden
        let current_page = document.location.href.toString();
        if (current_page.endsWith("/")) {
            current_page += "index.html";
        }
        var links = Array.prototype.slice.call(this.querySelectorAll("a"));
        var l = links.length;
        for (var i = 0; i < l; ++i) {
            var link = links[i];
            var href = link.getAttribute("href");
            if (href && !href.startsWith("#") && !/^(?:[a-z+]+:)?\/\//.test(href)) {
                link.href = path_to_root + href;
            }
            // The "index" page is supposed to alias the first chapter in the book.
            if (link.href === current_page || (i === 0 && path_to_root === "" && current_page.endsWith("/index.html"))) {
                link.classList.add("active");
                var parent = link.parentElement;
                if (parent && parent.classList.contains("chapter-item")) {
                    parent.classList.add("expanded");
                }
                while (parent) {
                    if (parent.tagName === "LI" && parent.previousElementSibling) {
                        if (parent.previousElementSibling.classList.contains("chapter-item")) {
                            parent.previousElementSibling.classList.add("expanded");
                        }
                    }
                    parent = parent.parentElement;
                }
            }
        }
        // Track and set sidebar scroll position
        this.addEventListener('click', function(e) {
            if (e.target.tagName === 'A') {
                sessionStorage.setItem('sidebar-scroll', this.scrollTop);
            }
        }, { passive: true });
        var sidebarScrollTop = sessionStorage.getItem('sidebar-scroll');
        sessionStorage.removeItem('sidebar-scroll');
        if (sidebarScrollTop) {
            // preserve sidebar scroll position when navigating via links within sidebar
            this.scrollTop = sidebarScrollTop;
        } else {
            // scroll sidebar to current active section when navigating via "next/previous chapter" buttons
            var activeSection = document.querySelector('#sidebar .active');
            if (activeSection) {
                activeSection.scrollIntoView({ block: 'center' });
            }
        }
        // Toggle buttons
        var sidebarAnchorToggles = document.querySelectorAll('#sidebar a.toggle');
        function toggleSection(ev) {
            ev.currentTarget.parentElement.classList.toggle('expanded');
        }
        Array.from(sidebarAnchorToggles).forEach(function (el) {
            el.addEventListener('click', toggleSection);
        });
    }
}
window.customElements.define("mdbook-sidebar-scrollbox", MDBookSidebarScrollbox);
