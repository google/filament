# Web Tutorials

Here are the step-by-step tutorials to get you started with Filament on the Web.

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
    <a href="triangle.md" class="sample-card">
        <img src="../../images/web_sample_triangle.png" alt="triangle" />
        <span>triangle</span>
    </a>
    <a href="redball.md" class="sample-card">
        <img src="../../images/web_sample_redball.png" alt="redball" />
        <span>redball</span>
    </a>
    <a href="suzanne.md" class="sample-card">
        <img src="../../images/web_sample_suzanne.png" alt="suzanne" />
        <span>suzanne</span>
    </a>
</div>
