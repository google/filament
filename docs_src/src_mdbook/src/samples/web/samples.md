# Web Samples

Here are some additional standalone examples demonstrating Filament's capabilities in WebGL:

<style>
.sample-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(140px, 1fr));
    gap: 20px;
    margin-top: 20px;
}
.sample-card {
    display: flex;
    flex-direction: column;
    align-items: center;
    text-align: center;
    text-decoration: none;
    color: inherit;
    border: 1px solid var(--sidebar-bg);
    border-radius: 8px;
    padding: 15px;
    transition: transform 0.2s, box-shadow 0.2s;
    background-color: var(--bg);
}
.sample-card:hover {
    transform: translateY(-2px);
    box-shadow: 0 4px 8px rgba(0,0,0,0.1);
    text-decoration: none;
}
.sample-card img {
    border-radius: 4px;
    margin-bottom: 10px;
    width: 100px;
    height: 100px;
    object-fit: cover;
}

</style>

<div class="sample-grid">
    <a href="animation.md" class="sample-card">
        <img src="../../images/web_sample_animation.png" alt="animation" />
        <span>animation</span>
    </a>
    <a href="cube_fl0.md" class="sample-card">
        <img src="../../images/web_sample_cube_fl0.png" alt="cube_fl0" />
        <span>cube_fl0</span>
    </a>
    <a href="helmet.md" class="sample-card">
        <img src="../../images/web_sample_helmet.png" alt="helmet" />
        <span>helmet</span>
    </a>
    <a href="morphing.md" class="sample-card">
        <img src="../../images/web_sample_morphing.png" alt="morphing" />
        <span>morphing</span>
    </a>
    <a href="parquet.md" class="sample-card">
        <img src="../../images/web_sample_parquet.png" alt="parquet" />
        <span>parquet</span>
    </a>
    <a href="skinning.md" class="sample-card">
        <img src="../../images/web_sample_skinning.png" alt="skinning" />
        <span>skinning</span>
    </a>
</div>
